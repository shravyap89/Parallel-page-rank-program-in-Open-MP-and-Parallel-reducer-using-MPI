#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

int main(int argc, char *argv[]){
	
	struct key_value{
		int key;
		int value;
		struct key_value *next;
	} *start = NULL, *end = start, *sample1 = start, *sample2;

	int myrank, mysize;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &mysize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Status status;
	
	int Number_processors = 4;

	int key_invalid = -1;

	int i, j, index1, Number_keyvaluepair = 0, Scatter_Data_size = 0, key, value, *data, *data2, min_key = key_invalid, max_key = key_invalid, Range_key, *Brodcastall_info, **Prep_data_output, **Prep_data_input, *data1;
	char string1[100], string2[100], delim[2] = ",";
	FILE *file;

	Brodcastall_info = (int *)malloc(sizeof(int) * 5);

	
	if(myrank == 0){
		file =fopen("100000_key-value_pairs.csv", "r");
	
		if(!file){
			printf("File not available");	
		}
		
		fscanf(file,"%s,%s",string1, string2);
		i = 0;
		fscanf(file, "%d,%d", &key, &value);
		min_key = key;
		max_key = key;
		do{
			sample1 = (struct key_value *) malloc (sizeof(struct key_value));
			if(start == NULL)
				start = sample1;	
			sample1->key = key;
			sample1->value = value;
			sample1->next = NULL;
			if(end == NULL)
				end = sample1;
			else
				end->next = sample1;
			end = sample1;
			Number_keyvaluepair = Number_keyvaluepair + 1;
			
			if(key < min_key)
				min_key = key;
			if(key > max_key)
				max_key = key;
		}while(fscanf(file, "%d,%d", &key, &value) ==  2);
		
		fclose(file);

		Range_key = max_key - min_key + 1;
		
		if((Number_keyvaluepair%Number_processors) != 0)	
			Scatter_Data_size = ((Number_keyvaluepair/Number_processors) + 1) * Number_processors * 2;
		else if((Number_keyvaluepair%Number_processors) == 0)
			Scatter_Data_size = Number_keyvaluepair * 2;
		
		while(key_invalid >= min_key)
			key_invalid--;

		Brodcastall_info[0] = Scatter_Data_size;
		Brodcastall_info[1] = min_key;
		Brodcastall_info[2] = max_key;
		Brodcastall_info[3] = Range_key;
		Brodcastall_info[4] = key_invalid;

		data = (int *) malloc(sizeof(int) * Scatter_Data_size);
				
		sample1 = start;
		i = 0;
		while(sample1 != NULL){
			data[i] = sample1->key;
			data[i + 1] = sample1->value;
			sample2 = sample1;		
			sample1 = sample1->next;
			free(sample2);
			i = i + 2;
		}
	}

	//BROADCAST
	MPI_Bcast(Brodcastall_info, 5, MPI_INT, 0, MPI_COMM_WORLD);
	
	Scatter_Data_size = Brodcastall_info[0];
	min_key = Brodcastall_info[1];
	max_key = Brodcastall_info[2];
	Range_key = Brodcastall_info[3];
	key_invalid= Brodcastall_info[4];

	data2 = (int *) malloc ( sizeof(int) * (Scatter_Data_size/Number_processors) );

	//SCATTERING 
	MPI_Scatter(data, (Scatter_Data_size/Number_processors), MPI_INT, data2, (Scatter_Data_size/Number_processors), MPI_INT, 0, MPI_COMM_WORLD);
	
	if(myrank ==0)	
		free(data);	

	//FIRST LOCAL REDUCTION
	for(i=0;i<(Scatter_Data_size/Number_processors);i=i+2){
		for(j=0;j<i;j=j+2){
			if(data2[i] == data2[j]){
				data2[j+1] = data2[j+1] + data2[i+1];
				data2[i] = key_invalid;		
			}		
		}	
	}

	
	Prep_data_output = (int **) malloc(sizeof(int *) * Number_processors);
	for (i = 0; i < Number_processors; ++i)
	{
		Prep_data_output[i] = (int *) malloc(sizeof(int) * (Scatter_Data_size/Number_processors));
		if ( Prep_data_output[i] == NULL){
			printf("Failed to allocate memory");
			exit(1);
		}
	}
	
	for(i=0;i<Number_processors;i++){
		for(j=0;j<(Scatter_Data_size/Number_processors);j=j+2){
			Prep_data_output[i][j] = key_invalid;
		}	
	}
	
	for(i=0;i<(Scatter_Data_size/Number_processors);i=i+2){
		key = data2[i];
		j = key/(Range_key/Number_processors);
		Prep_data_output[j][i] = key;
		Prep_data_output[j][i+1] = data2[i+1];	
	}	
        
	Prep_data_input = (int **) malloc(sizeof(int *) * Number_processors);
	for (i = 0; i < Number_processors; i++)
	{
		Prep_data_input[i] = (int *) malloc(sizeof(int) * (Scatter_Data_size/Number_processors));
		if ( Prep_data_input[i] == NULL){
			printf("Failed to allocate memory");
			exit(1);
		}
	}	
	
	for(i=0;i<Number_processors;i++){
		for(j=0;j<(Scatter_Data_size/Number_processors);j=j+2){
			Prep_data_input[i][j] = key_invalid;
		}	
	}
	
	for(i=0;i<(Scatter_Data_size/Number_processors);i=i+2){
		key = data2[i];
		j = key/(Range_key/Number_processors);
		if(j == myrank){
			Prep_data_input[j][i] = data2[i];//Key
			Prep_data_input[j][i+1] = data2[i+1];//Value
		}
	}

	//AFTER LOCAL REDUCTION SENDING KEYS TO RESPECTIVE PROCESSORS INTENDED TO PROCESS THEM
	for(i=0;i<Number_processors;i++){
		if(i == myrank){
			for(j=0;j<Number_processors;j++){
				if(j != i){
					MPI_Send(Prep_data_output[j], (Scatter_Data_size/Number_processors), MPI_INT, j, 200, MPI_COMM_WORLD);
				}			
			}
		}
		else
		{
			MPI_Recv(Prep_data_input[i], (Scatter_Data_size/Number_processors), MPI_INT, i, 200, MPI_COMM_WORLD, &status);			
		}	
	}

	free(Prep_data_output);
	free(data2);
	
	data2 = (int *)malloc(sizeof(int) * Scatter_Data_size);
	for(i=0;i<Scatter_Data_size;i=i+2)
		data2[i] = key_invalid;

	index1 = 0;
	for(i=0;i<Number_processors;i++){
		for(j=0;j<(Scatter_Data_size/Number_processors);j=j+2){
			if(Prep_data_input[i][j] != key_invalid){
				data2[index1] = Prep_data_input[i][j];
				data2[index1+1] = Prep_data_input[i][j+1];
				index1 = index1 + 2;
			}						
		}	
	}
	free(Prep_data_input);

	//SECOND LOCAL REDUCTION
	for(i=0;i<index1;i=i+2){
		for(j=0;j<i;j=j+2){
			if(data2[i] == data2[j]){
				data2[j+1] = data2[j+1] + data2[i+1];
				data2[i] = key_invalid;		
			}		
		}	
	}

	
	//INVALID KEY-VALUE PAIRS REMOVAL
	index1 = 0;
	for(i=0;i<Scatter_Data_size;i=i+2)
		if(data2[i] != key_invalid)
			index1=index1+2;
	
	
	data = (int *)malloc(sizeof(int) * index1);
	for(i=0;i<index1;i=i+2)
		data[i] = key_invalid;
	j=0;
	for(i=0;i<Scatter_Data_size;i=i+2){
		if(data2[i] != key_invalid){
			data[j] = data2[i];
			data[j+1] = data2[i+1];
			j=j+2;
		}
	}

	Scatter_Data_size = j;
	//SORT BEFORE OUTPUT TO FILE
	for(i=0;i<Scatter_Data_size;i=i+2){
		for(j=2;j<Scatter_Data_size;j=j+2){
			if(data[j-2] > data[j]){
				key = data[j];
				value = data[j+1];
				data[j] = data[j-2];
				data[j+1] = data[j-1];
				data[j-2] = key;
				data[j-1] = value;
			}
		}	
	}
	

	MPI_Send(&Scatter_Data_size, 1, MPI_INT, 0, 300, MPI_COMM_WORLD);
	MPI_Send(data, Scatter_Data_size, MPI_INT, 0, 400, MPI_COMM_WORLD);
	
	
	if(myrank == 0){
		file = fopen("Output_Task2.txt","w");
		for(i = 0 ; i < Scatter_Data_size ; i=i+2)
		{
			fprintf(file, "%d,%d\n", data[i], data[i+1]);
		}
		for(i=1;i<Number_processors;i++){
			MPI_Recv(&Scatter_Data_size, 1, MPI_INT, i, 300, MPI_COMM_WORLD, &status);
			free(data);
			data = (int *) malloc(sizeof(int) * Scatter_Data_size);
			MPI_Recv(data, Scatter_Data_size, MPI_INT, i, 400, MPI_COMM_WORLD, &status);
			for(j = 0 ; j < Scatter_Data_size ; j=j+2)
			{
				fprintf(file, "%d,%d\n", data[j], data[j+1]);
			}
		}
		free(data);
		fclose(file);
	}

	MPI_Finalize();
	return 0;
}
