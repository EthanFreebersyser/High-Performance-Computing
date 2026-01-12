#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CLOCKWISE   1
#define COUNTERCLK  2

int main(int argc, char *argv[]){
    int ierr;
    int rank, size;
    int src, dest;
    unsigned long iter; 
    unsigned long count = 0;
    int i;

    MPI_Status status;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &size);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand((unsigned int)time(NULL) * rank);

    if (rank == 0){
        iter = (long)atoi(argv[1]);  
    } 

    MPI_Bcast(&iter, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD) ;

    for (i = 0; i < iter; i++){
        double x = (double)rand()  / ((double) RAND_MAX + 1.0);
        double y = (double)rand()  / ((double) RAND_MAX + 1.0);

        if (x*x + y*y < 1){
            count++;
        }
    }
    
    int tmpCount;
    MPI_Reduce(&count, &tmpCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    count = tmpCount;

    if (rank == 0){
        double pi = (double)4*((double)count/((double)size * (double)iter));

        printf("Approx of pi: %.9f\n", pi);
    }

    MPI_Finalize();
    return 0;
}