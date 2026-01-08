#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define CLOCKWISE   1
#define COUNTERCLK  2

int main(int argc, char *argv[]){
    int ierr;
    int rank, size;
    int buf[3]; //[0] target, [1] dir, [2] count
    int src, dest;

    MPI_Status status;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &size);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int cw = (rank + 1) % size;
    int ccw = (rank - 1 + size) % size;

    if (rank == 0){
        buf[0] = rand() % size;

        //Determine the Dir
        if (rand() % 2 == 0){
            buf[1] = CLOCKWISE;
        } else {
            buf[1] = COUNTERCLK;
        }

        //grab the Count
        buf[2] = atoi(argv[1]);

        //Determine the dest
        if (buf[1] == CLOCKWISE){
            dest = cw;
        } else {
            dest = ccw;
        }

        printf("0: Sending %s to Rank %d (target %d, count %d)\n",
            (buf[1] == CLOCKWISE) ? "clockwise" : "counterclockwise",
            dest, buf[0], buf[2]);

        MPI_Send(buf, 3, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }

    while(1){
        MPI_Recv(buf, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        src = status.MPI_SOURCE;

        if (buf[2] <= 0){
            //Determine the dest
            if (buf[1] == CLOCKWISE){
                dest = cw;
            } else {
                dest = ccw;
            }

            printf("%d: Recvd TERMINATE from %d; forwarding to %d and exiting\n",
                   rank, src, dest);

            MPI_Send(buf, 3, MPI_INT, dest, 0, MPI_COMM_WORLD);
            break;
        }

        printf("%d: Recvd msg from %d (target %d, count %d)\n", rank, src, buf[0], buf[2]);

        if (buf[0] == rank){
            printf("%d: It is a message for me!!!\n", rank);

            buf[0] = rand() % rank;

            //Determine the Dir
            if (buf[1] == CLOCKWISE){
                buf[1] = COUNTERCLK;
            } else {
                buf[1] = CLOCKWISE;
            }

            //decrease the Count
            buf[2]--;

            //Determine the dest
            if (buf[1] == CLOCKWISE){
                dest = cw;
            } else {
                dest = ccw;
            }

            printf("%d: Sending %s to Rank %d (target %d, count %d)\n",
                   rank,
                   (buf[1] == CLOCKWISE) ? "clockwise" : "counterclockwise",
                   dest, buf[0], buf[2]);

            MPI_Send(buf, 3, MPI_INT, dest, 0, MPI_COMM_WORLD);
        
        } else {
            //Determine the dest
            if (buf[1] == CLOCKWISE){
                dest = cw;
            } else {
                dest = ccw;
            }

            printf("%d: Forwarding %s to Rank %d (target %d, count %d)\n",
                   rank,
                   (buf[1] == CLOCKWISE) ? "clockwise" : "counterclockwise",
                   dest, buf[0], buf[2]);

            MPI_Send(buf, 3, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}