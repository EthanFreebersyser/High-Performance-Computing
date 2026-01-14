#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

char **board[2];
int ROWSIZ, COLSIZ, numOfLivingCells;

// Function declarations
void initBoard(int rowsize, int colsize);
void displayBoard(int type, char **bp, int rank, int size, int global_colsize);
int* loadData();
void fillboard(int *liveCells, int rank, int size, int global_colsize);

/*
 *  Fill the edges of the board with data to make a
 *  periodic buffer ("wraparound")
 */
void
periodicBuffer(char **theboard, char *rightcol, char *leftcol)
{
	//Copy Top and Bottom Rows
	int i;
	for(i = 0; i < COLSIZ; i++){
		theboard[0][i] = theboard[ROWSIZ-2][i];
		theboard[ROWSIZ-1][i] = theboard[1][i];
	}

	//Copy Left and Right Columns from halo buffers
	for(i = 0; i < ROWSIZ; i++){
		theboard[i][0] = leftcol[i];
		theboard[i][COLSIZ-1] = rightcol[i];
	}
} // periodicBuffer

/*
 *  newLife - compute the "life" for cell at [row][col]
 *  		using data from oldboard and putting the
 *  		result (the next generation) into the newboard
 */
void
newLife(char **newboard, char **oldboard, int r, int c){
	int i, j;
	int liveCounter = 0;

	//check all spaces around (r,c) for a live cell
	//add to liveCounter if there is a live space nearby
	for (i = -1; i <= 1; i++){
		for (j = -1; j <= 1; j++){
			if (i == 0 && j == 0){
				continue;
			}
			if(oldboard[r+i][c+j] == 1){
				liveCounter++;
			}
		}
	}

	//apply rules to determine the next generation
	if(oldboard[r][c] == 1){
		if(liveCounter < 2){
			newboard[r][c] = 0; 
		} else if (liveCounter == 2 || liveCounter == 3){
			newboard[r][c] = 1; 
		} else if (liveCounter > 3){
			newboard[r][c] = 0;
		}
	} else {
		if (liveCounter == 3){
			newboard[r][c] = 1; 
		} else {
			newboard[r][c] = 0;
		}
	}
} // newLife

void getHaloCols(int rank, int size, char *colBuf, char *rightcol, char *leftcol){
	MPI_Status status;
	int left = (rank - 1 + size) % size;
	int right = (rank + 1) % size;
	int i;
	
	// Exchange Right Cols send right and receive left
	// Evens send first, odds receive first
	if (rank % 2 == 0) {
		//send right 
		for (i = 0; i < ROWSIZ; i++){
			colBuf[i] = board[0][i][COLSIZ - 2];
		}
		MPI_Send(colBuf, ROWSIZ, MPI_CHAR, right, 0, MPI_COMM_WORLD);
		//receive left
		MPI_Recv(leftcol, ROWSIZ, MPI_CHAR, left, 0, MPI_COMM_WORLD, &status);
	} else {
		//receive right
		MPI_Recv(leftcol, ROWSIZ, MPI_CHAR, left, 0, MPI_COMM_WORLD, &status);

		//send left 
		for (i = 0; i < ROWSIZ; i++){
			colBuf[i] = board[0][i][COLSIZ - 2];
		}
		MPI_Send(colBuf, ROWSIZ, MPI_CHAR, right, 0, MPI_COMM_WORLD);
	}

	// Exchange left Cols send right and receive left
	// Odds send first, evens receive first
	if (rank % 2 == 1) {
		//send left 
		for (i = 0; i < ROWSIZ; i++){
			colBuf[i] = board[0][i][1];
		}
		MPI_Send(colBuf, ROWSIZ, MPI_CHAR, left, 1, MPI_COMM_WORLD);
		//receive right
		MPI_Recv(rightcol, ROWSIZ, MPI_CHAR, right, 1, MPI_COMM_WORLD, &status);
	} else {
		//receive left
		MPI_Recv(rightcol, ROWSIZ, MPI_CHAR, right, 1, MPI_COMM_WORLD, &status);
	
		//send right 
		for (i = 0; i < ROWSIZ; i++){
			colBuf[i] = board[0][i][1];
		}
		MPI_Send(colBuf, ROWSIZ, MPI_CHAR, left, 1, MPI_COMM_WORLD);
	}
}

int
main(int argc, char **argv)
{
    int row,col, i;
	int *boardSize = malloc((size_t)(3 * sizeof(int))); //0: rows, 1: cols 2: numOfLivingCells
    int gen, genOut;
	int ierr;
    int rank, size;
	int *liveCells = NULL;
	int global_ROWSIZ, global_COLSIZ;

    MPI_Status status;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &size);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		genOut = atoi(argv[1]);
		
		// read inputfile and init array
    	liveCells = loadData();
		boardSize[0] = ROWSIZ; 
		boardSize[1] = COLSIZ; 
		boardSize[2] = numOfLivingCells;
	}

	// Broadcast board dimensions and number of living cells
    MPI_Bcast(boardSize, 3, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&genOut, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Set local dimensions
	global_ROWSIZ = boardSize[0];
	global_COLSIZ = boardSize[1];
	numOfLivingCells = boardSize[2];
	
	// Each rank handles a range of columns
	ROWSIZ = global_ROWSIZ;
	COLSIZ = global_COLSIZ / size;

	// Add space for the buffers (+2 for halo)
    ROWSIZ += 2;
    COLSIZ += 2;

	/* now allocate it */
    initBoard(ROWSIZ, COLSIZ);

	//make space for liveCells on other ranks
	if (rank != 0) {
		liveCells = malloc((size_t)(numOfLivingCells * 2 * sizeof(int)));
	}

	// Broadcast all live cells to all ranks
	MPI_Bcast(liveCells, numOfLivingCells * 2, MPI_INT, 0, MPI_COMM_WORLD);

	// Each rank fills its local board section
	fillboard(liveCells, rank, size, global_COLSIZ);

	// Allocate buffers for halo exchange (use char, not int)
	char *colBuf = malloc((size_t)(ROWSIZ * sizeof(char)));
	char *rightcol = malloc((size_t)(ROWSIZ * sizeof(char)));
	char *leftcol = malloc((size_t)(ROWSIZ * sizeof(char)));

    for(gen=0; gen < genOut; gen++) {

		// Exchange halo columns with neighbors
		getHaloCols(rank, size, colBuf, rightcol, leftcol);

		// Fill periodic buffer with halo data
		periodicBuffer(board[(gen & 1)], rightcol, leftcol);

		// Compute next generation
		for (row=1; row < ROWSIZ-1; row++) {
			for (col=1; col < COLSIZ-1; col++) {
			if (gen & 1) {
				newLife(board[0], board[1], row, col);
			} else {
				newLife(board[1], board[0], row, col);
			}
			} // next col
		} // next row

    } // next gen

	// Count Live cells in this rank
	int localLiveCount = 0;
	for (row = 1; row < ROWSIZ-1; row++) {
		for (col = 1; col < COLSIZ-1; col++) {
			if (board[1-(gen&1)][row][col] == 1) {
				localLiveCount++;
			}
		}
	}

	// Gather live cell counts
	int *liveCounts = NULL;
	if (rank == 0) {
		liveCounts = malloc((size_t)(size * sizeof(int)));
	}
	MPI_Gather(&localLiveCount, 1, MPI_INT, liveCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Get and send live cell locations to rank 0
	int *localLiveCells = malloc((size_t)(localLiveCount * 2 * sizeof(int)));
	int idx = 0;
	int col_offset = (COLSIZ - 2) * rank;

	for (row = 1; row < ROWSIZ-1; row++) {
		for (col = 1; col < COLSIZ-1; col++) {
			if (board[1-(gen&1)][row][col] == 1) {
				localLiveCells[idx++] = row - 1; // Remove halo offset
				localLiveCells[idx++] = (col - 1) + col_offset; // Map to global col
			}
		}
	}

	// Gather live cell locations
	int *recvCounts = NULL;
	int *displs = NULL;
	int *allLiveCells = NULL;

	if (rank == 0) {
		recvCounts = malloc((size_t)(size * sizeof(int)));
		displs = malloc((size_t)(size * sizeof(int)));
		int totalLiveCells = 0;
		for (i = 0; i < size; i++) {
			recvCounts[i] = liveCounts[i] * 2; //each cell has row and col
			displs[i] = totalLiveCells;
			totalLiveCells += recvCounts[i];
		}
		allLiveCells = malloc((size_t)(totalLiveCells * sizeof(int)));	
	}

	MPI_Gatherv(localLiveCells, localLiveCount * 2, MPI_INT, 
		allLiveCells, recvCounts, displs, MPI_INT, 
		0, MPI_COMM_WORLD);

	// Rank 0 outputs final live cells in input file format
	if (rank == 0) {
			int totalCells = 0;
			for (i = 0; i < size; i++) {
				totalCells += liveCounts[i];
			}

		// Create output filename based on number of generations
		char filename[256];
		snprintf(filename, sizeof(filename), "gen%d.data", genOut);
		
		FILE *outfile = fopen(filename, "w");

		// Output in format: rows cols numLiveCells
		fprintf(outfile, "%d %d %d\n", global_ROWSIZ, global_COLSIZ, totalCells);

		// Output coordinates
		for (i = 0; i < totalCells * 2; i += 2) {
			fprintf(outfile, "%d %d\n", allLiveCells[i], allLiveCells[i+1]);
		}
		
		fclose(outfile);
		fprintf(stderr, "Output written to %s\n", filename);
		
		free(liveCounts);
		free(recvCounts);
		free(displs);
		free(allLiveCells);
	}

	// Cleanup
	free(colBuf);
	free(rightcol);
	free(leftcol);
	free(localLiveCells);
	free(liveCells);
	free(boardSize);

	MPI_Finalize();
	return 0;
} // main