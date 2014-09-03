Task 1: Parallel Page rank program in open mp for Ego-Facebook.

Compilation: Compilation steps have been included in README.txt.
Brief Description of tasks completed and how I have them achieved them via code in C
language:
Step 1: Calculate the total number of distinct Nodes in given input data file.
For this input file is read and a simple logic is executed to find the min and max node numbers
present in input file. This gives the "N".I also exclude the first entry i.e. key,value in input file.

Step 2:"NXN" matrix memory allocation using Malloc.
Now that "N" is available from step1 we have to create a 2D array with N rows and N columns
which is called web matrix ("M") for the input data file. This is initialized to 1. Also,rank vector
"vector" is initialized to (1/N) with N rows and 1 column.

Step 3: Web-matrix creation.
For every edge distinct node in input file, Total number of outgoing edges for that particular
node is calculated. That is for to say if there exits an edge 0 to 239 and 0 to
45, then sum of outgoing edges for node 0 is 2.Likewise, Outgoing links for node is stored in
an array.
Using the values calculated above,M matrix is populated with the data such that if if node x
links to y then Matrix[y][x]= Matrix[y][x]/r[x] where r[x] is the sum of outgoing links of node 0.

Step 4: Iterative loop for Page rank calculations and Pragma omp usage for parallel
processing of the data by multiple threads.
For calculation of page rank following logic is used:
r1=Mr
r2=Mr1
. .
. .
. .
r(k+1)=Mr(k)
Web-matrix “Matrix” is multiplied with rank vector and then iteratively the calculations are done
to check for convergence of page ranks.
Herein, as this is the most expensive "for" loop omp parallel for is used here. The shared
variable which are to be used by all threads are defined.

Step 5: Dead links.
In order to take care of dead links, a factor of d=0.85 is considered. And every time rank vector
is calculated following steps are done:
--> sum += (M[x][y] * vector[y])
--> r[x] = ( (1-d) / N ) + (d * sum)
--> rankvector[x] = r[x];
That means product of initial rank vector and web-matrix created is multiplied with factor d and
added to (( (1-d) / N )). Thus this takes care of the dead links. Subsequent iterations are done
with new value of rankvector[x] each time.

Step 6:Output and Iteration value:34
Iterations are performed till converging data appears done with help of Iteration counter and
printed to a file. Total number of Iterations I have done is :34.After 34th iteration, there is no
significant change in page rank for higher value of iteration numbers. Therefore I have stopped
at this point.

--------------------------------------------------------------------------------------------------------------------------------------
Task 2: Parallel reducer using MPI
This task reduces multiple key value pairs in parallel provided in the input file using the MPI .

STEP 1: Reading key value pairs from input.
Key-value pairs were read from file ” 100000_key-value_pairs.csv” by processor with rank = 0
and stored in a data structure created in memory using struct.

STEP2: Number of processor and Number of key value pairs.
If number of processors does not exactly divide number of key-value pairs then invalid keys are
generated so that total data now is divisible by number of processors. .Invalid key can be an
number lesser than the minimum key received from the input file .

STEP 3: Broadcast data size to processed by each processor and Scatter relevant data
to each processor.
After using MPI_Bcase to communicate size of data information the complete data set was
divided equally among processors and communicated to different processors

STEP 4: First Reduction
Local reduction of key value pair is done so that each processor has one key at most once.

STEP 5: MPI SEND AND RECEIVE to process one key in only one processor.
For final reduction all keys with same Key ID need to brought on a single processors and
distribution of keys to be processed by each processor has to be taken taken care of. Logic
used here is for 4 processors if 100 key-value pair exist then, those with key ID 0-24 are
processed by one processors 25-49 by next and so on.MPI_Send() and MPI_Recv() are used
for this.

STEP 6: Second Reduction and writing output to file
Second reduction is performed by each processor on the data chunk to produce distinct keyvalue
pairs . Data is then sorted according to keys in increasing order and communicated back
to the processor with rank 0 and it’s data to the output file and then the data received from
other processors in increasing order of rank thus maintaining the overall order of keys in the
output file .