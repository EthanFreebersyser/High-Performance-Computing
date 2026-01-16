#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <sys/stat.h>

/*
 * Forest Fire Sim - Stage 1 - get Halo Exchange working
 */

//goal: grid[switch][colums][rows]; 2 grids for swapping generations
char **grid[2];

// all dimentions are square; #rows = #cols
int simsiz = 2000;	// how big is the whole sim (each side)
int sqrtworld;
int gridsiz;		// how big is our piece (each side)
int halogrsiz;	// how big is our piece with the halo
int topN, bottomN, leftN, rightN; //ranks of the Neighbors
int rank, world;
int i,j,k;

/*
 * create both grids: [0] and [1]
 */
void  initGrid(int colsize, int rowsize){
    int b;
    for(b=0;b<=1;b++) {	// for each grid

        /* allocate the row pointers */
        grid[b] = malloc((size_t)(rowsize * sizeof(char *)));

        /* allocate space for the grid data */
	    grid[b][0] = calloc((size_t) rowsize * (size_t)colsize, sizeof(char));

	    /* assign all the row pointers */
        for (i=1; i < rowsize; i++) {
            grid[b][i] = grid[b][i-1] + colsize;
        }
    }

} // initGrid

/*
 * fillerup  - fill the grid with data
 *     gridid - which of the two grids to fill
 *     rcstart - starting position within the grid
 *     rcend - ending position within the grid
 *     value - the value to put in those locations
 *
 *     remember: grid is square, so start, end are same for row and col
 */
void fillerup(int gridid, int rcstart, int rcend, char value) {
    int r, c;
    for (r = rcstart; r <= rcend; r++) {
        for (c = rcstart; c <= rcend; c++) {
            grid[gridid][r][c] = value;
        }
    }
} // fillerup

void sendRight(){
    // Send to right neighbor (if we have one)
    if (rank % sqrtworld != (sqrtworld - 1)){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        for (i = 0; i < halogrsiz; i++){
            colbuf[i] = grid[0][i][gridsiz]; // rightmost data column
        }
        MPI_Send(colbuf, halogrsiz, MPI_CHAR, rightN, 0, MPI_COMM_WORLD);
        free(colbuf);
    }
}

void recvRight(){
    // Receive from right neighbor (if we have one)
    if (rank % sqrtworld != (sqrtworld - 1)){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        MPI_Recv(colbuf, halogrsiz, MPI_CHAR, rightN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < halogrsiz; i++){
            grid[0][i][halogrsiz-1] = colbuf[i]; // right halo column
        }
        free(colbuf);
    }
}

void sendLeft(){
    // Send to left neighbor (if we have one)
    if(rank % sqrtworld != 0){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        for (i = 0; i < halogrsiz; i++){
            colbuf[i] = grid[0][i][1]; // leftmost data column
        }
        MPI_Send(colbuf, halogrsiz, MPI_CHAR, leftN, 0, MPI_COMM_WORLD);
        free(colbuf);
    }
}

void recvLeft(){
    // Receive from left neighbor (if we have one)
    if(rank % sqrtworld != 0){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        MPI_Recv(colbuf, halogrsiz, MPI_CHAR, leftN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < halogrsiz; i++){
            grid[0][i][0] = colbuf[i]; // left halo column
        }
        free(colbuf);
    }
}

void rightLeftExch() {
    if ((rank % sqrtworld) % 2 == 0) { // even column ranks
        sendRight();
        recvRight();
        sendLeft();
        recvLeft();
    } else {		   // odd column ranks
        recvLeft();
        sendLeft();
        sendRight();
        recvRight();
    }

} // rightLeftExch

void sendDown(){
    // Send to bottom neighbor (if we have one)
    if (rank / sqrtworld != (sqrtworld - 1)){
MPI_Send(grid[0][gridsiz], halogrsiz, MPI_CHAR, bottomN, 0, MPI_COMM_WORLD);
    }
}

void recvDown(){
    // Receive from bottom neighbor (if we have one)
    if (rank / sqrtworld != (sqrtworld - 1)){
MPI_Recv(grid[0][halogrsiz-1], halogrsiz, MPI_CHAR, bottomN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void sendUp(){
    // Send to top neighbor (if we have one)
    if(rank / sqrtworld != 0){
    MPI_Send(grid[0][1], halogrsiz, MPI_CHAR, topN, 0, MPI_COMM_WORLD);
    }
}

void recvUp(){
    // Receive from top neighbor (if we have one)
    if(rank / sqrtworld != 0){
    MPI_Recv(grid[0][0], halogrsiz, MPI_CHAR, topN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void downUpExch() {
     if ((rank / sqrtworld) % 2 == 0) { // even row ranks
        sendDown();
        recvDown();
        sendUp();
	    recvUp();
    } else {		   // odd row ranks
	    recvUp();
        sendUp();
        recvDown();
        sendDown();
    }
} // downUpExch

void haloExchange() {
    rightLeftExch();
    downUpExch();
} // haloExchange


int sumLRHC(){
    int sum = 0;  // Initialize to 0!
    for (i = -1; i < 2; i++){
        for (j = -1; j < 2; j++){
            sum += grid[0][gridsiz+i][gridsiz+j];
        }
    }
    return sum;
}

/*
 * Gather all grids to rank 0 and output combined board to a single file
 */
int outputToFile(){
    char filename[512];
    int gens = 1;
    int r, src_rank;
    
    // Create a contiguous buffer for this rank's grid data no halo
    char *localData = malloc((size_t)(gridsiz * gridsiz * sizeof(char)));
    
    // Pack local grid data into contiguous buffer no halo
    for (i = 0; i < gridsiz; i++){
        for (j = 0; j < gridsiz; j++){
            localData[i * gridsiz + j] = grid[0][i + 1][j + 1];
        }
    }
    
    if (rank == 0) {
        char *fullBoard = malloc((size_t)(simsiz * simsiz * sizeof(char)));
        
        char *recvBuf = malloc((size_t)(gridsiz * gridsiz * sizeof(char)));
        
        //Copy rank 0's into the full board
        int rank0_row = 0;
        int rank0_col = 0;
        for (i = 0; i < gridsiz; i++){
            for (j = 0; j < gridsiz; j++){
                int global_row = rank0_row * gridsiz + i;
                int global_col = rank0_col * gridsiz + j;
                fullBoard[global_row * simsiz + global_col] = localData[i * gridsiz + j];
            }
        }
        
        // Receive data from all other ranks and place into full board
        for (src_rank = 1; src_rank < world; src_rank++){
            MPI_Recv(recvBuf, gridsiz * gridsiz, MPI_CHAR, src_rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  

	        //  Find where this rank's data goes in the global grid
            int src_row = src_rank / sqrtworld;  // which row of ranks
            int src_col = src_rank % sqrtworld;  // which column of ranks
            
            // Copy data into the correct position in fullBoard
            for (i = 0; i < gridsiz; i++){
                for (j = 0; j < gridsiz; j++){
                    int global_row = src_row * gridsiz + i;
                    int global_col = src_col * gridsiz + j;
                    fullBoard[global_row * simsiz + global_col] = recvBuf[i * gridsiz + j];
                }
            }
        }
        
        // Write the combined board to a single file
        snprintf(filename, sizeof(filename), "board_gen%d.data", gens);
	    FILE *fp = fopen(filename, "w");

    for (i = 0; i < simsiz; i++){
        for (j = 0; j < simsiz; j++){
            if (j > 0){
                fprintf(fp, " "); // space between cols
            }
            fprintf(fp, "%d", fullBoard[i * simsiz + j]);
        }
        fprintf(fp, "\n"); // newline after each row
    }

    fclose(fp);

        free(recvBuf);
        free(fullBoard);
        
    } else {
        // All other ranks send their data to rank 0
        MPI_Send(localData, gridsiz * gridsiz, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
    }
    
    free(localData);
    return 0;
}

int main(int argc, char **argv){
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc > 1) { simsiz = atoi(argv[1]); }
    sqrtworld = sqrt(world);
    gridsiz = simsiz / sqrtworld;
    halogrsiz = gridsiz+2;

    initGrid(halogrsiz, halogrsiz);
    fillerup(0, 1, gridsiz, rank);

    //Set neighbors
    topN = rank - sqrtworld;
    bottomN = rank + sqrtworld;
    leftN =  rank - 1;
    rightN = rank + 1;

    // Halo Exchange
    haloExchange();

    // Sum up the LRHC
    int sum = sumLRHC();

    // MPI_Reduce to rank 0 of all LRHC values
    int finalSum;

    MPI_Reduce(&sum, &finalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0){
        printf("Final Sum: %d\n", finalSum);
    }

    outputToFile();
    
    MPI_Finalize();
    return 0;

} // main