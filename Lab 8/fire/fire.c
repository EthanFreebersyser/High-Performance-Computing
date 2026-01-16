#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include <sys/stat.h>


//grid[switch][colums][rows]; 2 grids for swapping generations
char **grid[2];

// all dimentions are square; #rows = #cols
int simsiz = 30000;	// how big is the whole sim (each side)
int sqrtworld;
int gridsiz;		// how big is our piece (each side)
int halogrsiz;	// how big is our piece with the halo
int topN, bottomN, leftN, rightN; //ranks of the Neighbors
int rank, world;
int i,j,k;
double treeDensity;
double fireProb = 0.00000001;
double spreadProb = 0.33;

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
void fillerup(int gridid, int rcstart, int rcend) {
    int r, c;
    double randVal;
    int randAge;

    for (r = rcstart; r <= rcend; r++) {
        for (c = rcstart; c <= rcend; c++) {
            //pick a num between 0-1
            randVal = (double)rand() / RAND_MAX;

            if (randVal < treeDensity){
                randAge = (rand() % 5) + 1;

                if(randAge > 3) randAge = 3;

                grid[gridid][r][c] = (char)randAge;
            } else {
                // No tree
                grid[gridid][r][c] = 0;
            }
        }
    }
} // fillerup

void startFire(int gridid, int rcstart, int rcend){
    int r, c;
    double randVal;

    for (r = rcstart; r <= rcend; r++) {
        for (c = rcstart; c <= rcend; c++) {  
            if (grid[gridid][r][c] > 0) {
                randVal = (double)rand() / RAND_MAX;

                if (randVal < fireProb) {
                     grid[gridid][r][c] = -grid[gridid][r][c];
                }
            }
        }
    }  
} // startFire

void sendRight(int gid){
    // Send to right neighbor (if we have one)
    if (rank % sqrtworld != (sqrtworld - 1)){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        for (i = 0; i < halogrsiz; i++){
            colbuf[i] = grid[gid][i][gridsiz]; // rightmost data column
        }
        MPI_Send(colbuf, halogrsiz, MPI_CHAR, rightN, 0, MPI_COMM_WORLD);
        free(colbuf);
    }
}

void recvRight(int gid){
    // Receive from right neighbor (if we have one)
    if (rank % sqrtworld != (sqrtworld - 1)){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        MPI_Recv(colbuf, halogrsiz, MPI_CHAR, rightN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < halogrsiz; i++){
            grid[gid][i][halogrsiz-1] = colbuf[i]; // right halo column
        }
        free(colbuf);
    }
}

void sendLeft(int gid){
    // Send to left neighbor (if we have one)
    if(rank % sqrtworld != 0){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        for (i = 0; i < halogrsiz; i++){
            colbuf[i] = grid[gid][i][1]; // leftmost data column
        }
        MPI_Send(colbuf, halogrsiz, MPI_CHAR, leftN, 0, MPI_COMM_WORLD);
        free(colbuf);
    }
}

void recvLeft(int gid){
    // Receive from left neighbor (if we have one)
    if(rank % sqrtworld != 0){
        char *colbuf = malloc((size_t)(halogrsiz * sizeof(char)));
        MPI_Recv(colbuf, halogrsiz, MPI_CHAR, leftN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (i = 0; i < halogrsiz; i++){
            grid[gid][i][0] = colbuf[i]; // left halo column
        }
        free(colbuf);
    }
}

void rightLeftExch(int gid) {
    if ((rank % sqrtworld) % 2 == 0) { // even column ranks
        sendRight(gid);
        recvRight(gid);
        sendLeft(gid);
        recvLeft(gid);
    } else {		   // odd column ranks
        recvLeft(gid);
        sendLeft(gid);
        sendRight(gid);
        recvRight(gid);
    }

} // rightLeftExch

void sendDown(int gid){
    // Send to bottom neighbor (if we have one)
    if (rank / sqrtworld != (sqrtworld - 1)){
MPI_Send(grid[gid][gridsiz], halogrsiz, MPI_CHAR, bottomN, 0, MPI_COMM_WORLD);
    }
}

void recvDown(int gid){
    // Receive from bottom neighbor (if we have one)
    if (rank / sqrtworld != (sqrtworld - 1)){
MPI_Recv(grid[gid][halogrsiz-1], halogrsiz, MPI_CHAR, bottomN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void sendUp(int gid){
    // Send to top neighbor (if we have one)
    if(rank / sqrtworld != 0){
    MPI_Send(grid[gid][1], halogrsiz, MPI_CHAR, topN, 0, MPI_COMM_WORLD);
    }
}

void recvUp(int gid){
    // Receive from top neighbor (if we have one)
    if(rank / sqrtworld != 0){
    MPI_Recv(grid[gid][0], halogrsiz, MPI_CHAR, topN, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void downUpExch(int gid) {
     if ((rank / sqrtworld) % 2 == 0) { // even row ranks
        sendDown(gid);
        recvDown(gid);
        sendUp(gid);
	    recvUp(gid);
    } else {		   // odd row ranks
	    recvUp(gid);
        sendUp(gid);
        recvDown(gid);
        sendDown(gid);
    }
} // downUpExch

void haloExchange(int gid) {
    rightLeftExch(gid);
    downUpExch(gid);
} // haloExchange

double density(int gridid, int rcstart, int rcend){
    int r,c;
    long treeCnt = 0;
    long totalCells = 0;

    for (r = rcstart; r <= rcend; r++) {
        for (c = rcstart; c <= rcend; c++) { 
            totalCells++;

            if(grid[gridid][r][c] > 0){
                treeCnt++;
            }
        }
    }

    if (totalCells == 0) return 0.0;
    return (double)treeCnt / (double)totalCells;
}

int nextGen(int srcGrid, int dstGrid, int rcstart, int rcend){
    int r,c;
    int changes = 0;
    char cellVal;
    char newVal;
    int burningN;
    double randVal;

    for (r = rcstart; r <= rcend; r++) {
        for (c = rcstart; c <= rcend; c++) {
            cellVal = grid[srcGrid][r][c];
            newVal = cellVal;

            if (cellVal < 0){

                newVal = cellVal + 1;
                changes++;
            } else if (cellVal == 0) {
                newVal = 0;
            } else {
                burningN = 0;

                // Only check neighbors if they're within valid data range
                if (r > 0 && grid[srcGrid][r - 1][c] < 0) burningN++;           // North
                if (r < halogrsiz-1 && grid[srcGrid][r + 1][c] < 0) burningN++; // South
                if (c < halogrsiz-1 && grid[srcGrid][r][c + 1] < 0) burningN++; // East
                if (c > 0 && grid[srcGrid][r][c - 1] < 0) burningN++;           // West

                for (i = 0; i < burningN; i ++){
                    randVal = (double)rand() / RAND_MAX;
                    if (randVal < spreadProb) {
                        newVal = -cellVal;
                        changes++;
                        break;
                    }
                }
            }

            grid[dstGrid][r][c] = newVal;
        }
    }

    return changes;
} // nextGen

/*
 * Gather all grids to rank 0 and output combined board to a single file
 */
int outputToFile(int gid, int gens){
    char filename[512];
    int r, src_rank;
    
    // Create a contiguous buffer for this rank's grid data no halo
    char *localData = malloc((size_t)(gridsiz * gridsiz * sizeof(char)));
    
    // Pack local grid data into contiguous buffer no halo
    for (i = 0; i < gridsiz; i++){
        for (j = 0; j < gridsiz; j++){
            localData[i * gridsiz + j] = grid[gid][i + 1][j + 1];
        }
    }
    
    if (rank == 0) {

        mkdir("data", 0755);

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
        snprintf(filename, sizeof(filename), "data/board_gen%d.data", gens);
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
    if (argc > 2) { treeDensity = atof(argv[2]); }
    if (argc > 3) { fireProb = atof(argv[3]); }
    
    sqrtworld = sqrt(world);
    gridsiz = simsiz / sqrtworld;
    halogrsiz = gridsiz+2;

     if (rank == 0) {
        printf("Simulation size: %d x %d\n", simsiz, simsiz);
        printf("Tree density: %f\n", treeDensity);
        printf("Processes: %d (%d x %d)\n", world, sqrtworld, sqrtworld);
        printf("Grid per rank: %d x %d (with halo: %d x %d)\n", gridsiz, gridsiz, halogrsiz, halogrsiz);
    }

    srand(time(NULL) + rank);

    initGrid(halogrsiz, halogrsiz);

    fillerup(0, 1, gridsiz);
    startFire(0, 1, gridsiz);

    //Set neighbors
    topN = rank - sqrtworld;
    bottomN = rank + sqrtworld;
    leftN =  rank - 1;
    rightN = rank + 1;

    int changing = 1;
    int localchange;
    int srcGrid = 0;
    int dstGrid = 1;
    int gen = 0;

    outputToFile(0, gen);

    double initDensity = density(0, 1, gridsiz);
    double totalInitDen;
    MPI_Reduce(&initDensity, &totalInitDen, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Initial forest density: %.4f%%\n", (totalInitDen / world) * 100.0);
        printf("Starting simulation...\n");
    }

    while (changing) {
        haloExchange(srcGrid);
        localchange = nextGen(srcGrid, dstGrid, 1, gridsiz);
        MPI_Allreduce(&localchange, &changing, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        gen++;

        int tmp = srcGrid;
        srcGrid = dstGrid;
        dstGrid = tmp;

        outputToFile(srcGrid, gen);

        if (rank == 0 && gen % 1 == 0) {
            printf("Generation %d: %d changes\n", gen, changing);
        }
    }
    double finalDensity = density(srcGrid,1,gridsiz);
    double totalFinDen;
    MPI_Reduce( &finalDensity, &totalFinDen, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double avgFinalDensity = totalFinDen / world;
        double avgInitDensity = totalInitDen / world;
        printf("\n=== Simulation Complete ===\n");
        printf("Total generations: %d\n", gen);
        printf("Final forest density: %.4f%%\n", avgFinalDensity * 100.0);
        printf("Forest remaining: %.2f%% of original\n", (avgFinalDensity / avgInitDensity) * 100.0);
    }
    
    MPI_Finalize();
    return 0;

} // main