#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h> 	

int main()
{
	struct Edge_Graph{
		int x;
		int y;
		struct Edge_Graph *nxt;
	} *start = NULL, *end = start, *sample1, *sample2;
        
	FILE *file1;
	int x, y, i, t, N, min_val, max_val, iterNum =34;
	char line [ 128 ], delim[2] = " ";
	double b = 0.85;
	
	file1 =fopen("facebook_combined.txt", "r");

	if (file1 == NULL){
		printf("FILE NOT PRESENT");	
		return 1;
	}
        // CALUCULATION OF THE TOTAL NUMBER OF NODES 
	while ( fgets ( line, sizeof line, file1 ) != NULL )
	{
		x = atoi(strtok (line,delim));
		y = atoi(strtok (NULL,delim));
		if(x < min_val)
			min_val = x;
		if(x > max_val)
			max_val = x;
		if(y < min_val)
			min_val = y;
		if(y > max_val)
			max_val = y;
		sample1 = (struct Edge_Graph *) malloc (sizeof(struct Edge_Graph));
		if(start == NULL)
			start = sample1;
		sample1->x = x;
		sample1->y = y;
		sample1->nxt = NULL;
		if(end != NULL)
			end->nxt = sample1;
		end = sample1;
	}
	
	fclose(file1);

	N = max_val - min_val + 1;
	
	double r[N], rankvector[N];
	double **Matrix;
	Matrix = (double**)malloc(sizeof(double*) * N);
	
     // MEMORY ALLOCATION FOR NXN matrix
	for (i = 0; i < N; ++i)
	{
		Matrix[i] = (double *) malloc(sizeof(double) * N);
		if ( Matrix[i] == NULL){
			printf("Failed to allocate memory");
			exit(1);
		}
	}
      
	for(x = 0 ; x < N ; x++){
		for(y=0;y<N;y++){
			Matrix[x][y] = 0.0;		
		}
		r[x] = 0;	
	}


	// INTIALIZING RANK VECTOR WITH 1.0/N
	for(x = 0 ; x < N ; x++){
		rankvector[x] = 1.0/N;	
	}
	
	//WEBMATRIX CREATION AND EDGE FACTOR INCLUSION
	sample1 = start;
	while(sample1 != NULL){
		x = sample1->x;
		y = sample1->y;
		Matrix[y][x] = 1;	
		Matrix[x][y] = 1;
		r[x]++;
		r[y]++;
		sample2 = sample1;
		sample1 = sample1->nxt;
		free(sample2);
	}
	
	for(x = 0 ; x < N ; x++){
		for(y = 0 ; y < N ; y++){
			Matrix[y][x] = Matrix[y][x]/r[x];		
		}
	}
	
	
        // Iterative rank calculation with the help of open mp parallelism
       #pragma omp parallel for default(none) shared(rankvector,M,temp) 
	for(t = 0 ; t < iterNum ; t++){	 
		for(x = 0 ; x < N ; x++){ 
                        double sum=0.0;
			for(y = 0 ; y < N ; y++){
				sum += (Matrix[x][y] * rankvector[y]);
			}
 		      //Deadlink taken care of with help of factor b=0.85
                      r[x] = ( (1-b) / N ) + (b * sum);	
		}

		for(x = 0 ; x < N ; x++)
			rankvector[x] = r[x];
	}
        
        //Output

	file1 = fopen("Output_Task1.txt","w");	

	for(x = 0 ; x < N ; x++)
	{
		fprintf(file1, "Page Rank for %d is : %lf\n", x, rankvector[x]);
	}

	fclose(file1);
	printf("Task Completed.\n");
	return 1;
}
