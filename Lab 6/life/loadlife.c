// The Game Board - with two boards: current and next-gen
//goal: board[switch][ROWSIZ][COLSIZ]; 2 boards for swapping generations

#include <stdio.h>
#include <stdlib.h>

extern char **board[2];
extern int ROWSIZ, COLSIZ, numOfLivingCells;

/*
 * create both boards: [0] and [1]
 */
void initBoard(int rowsize, int colsize)
{
    int b, i;

    for(b=0;b<=1;b++) {
        board[b] = malloc((size_t)(rowsize * sizeof(char *)));
        for (i=0; i < rowsize; i++) {
            board[b][i] = calloc((size_t)colsize, sizeof(char));
        }
    }

} // initBoard

/*
 * Display the current state of life - show live cells as a @, else blank
 *
 * pass in type=0 for raw x,y points;
 * non-0 for ascii board: (1=normal; 2=show halo)
*/
void
displayBoard(int type, char **bp, int rank, int size, int global_colsize)
{
    int row,col;
    int colfirst, rowfirst;
    int colsize, rowsize;

    if (type) {
	if (type >= 1) {	// just the board
	    rowfirst=1;
	    rowsize=ROWSIZ-1;
	    colfirst=1;
	    colsize=COLSIZ-1;
	} else {		// include the halo (debugging)
	    rowfirst=0;
	    rowsize=ROWSIZ;
	    colfirst=0;
	    colsize=COLSIZ;
	}
	// add a delimiter between successive boards (at the top)
	if (rank == 0) {
	    for (col=colfirst; col < colsize; col++) {
		printf("-");
	    }
	    printf("\n");
	}

	for (row=rowfirst; row < rowsize; row++) {
	    for (col=colfirst; col < colsize; col++) {
	      printf ("%c", bp[row][col]?'@':'.');  // live=='@'
	    }
	    if (rank == 0) {
		printf ("|\n");		// add an edge on the RHS
	    } else {
		printf("\n");
	    }
	}
	
    } else {				// i.e., type==0
	// Output global coordinates
	int local_cols = global_colsize / size;
	int col_offset = local_cols * rank;
	
	// never report on the halo
	if (rank == 0) {
	    printf ("%d %d\n", ROWSIZ-2, global_colsize);
	}
	for (row=1; row < ROWSIZ-1; row++) {
	    for (col=1; col < COLSIZ-1; col++) {
	      if(bp[row][col]) {
		  // Convert to global coordinates
		  int global_row = row - 1;
		  int global_col = (col - 1) + col_offset;
		  printf ("%d %d\n", global_row, global_col);
	      }
	    }
	}
    }

} // displayBoard

/*
 *  read inputfile - two integers per line; first line is size of board
 */
int* loadData()
{
    /* first, read in the board size */
    scanf("%d %d %d\n", &ROWSIZ, &COLSIZ, &numOfLivingCells);
    
	int *livingCells = malloc((size_t)(numOfLivingCells * 2 * sizeof(int)));
    
    /* now read in the existing live cells into board [0] */
	int i;
	for (i = 0; i < numOfLivingCells * 2; i+=2){
		scanf("%d %d", &livingCells[i], &livingCells[i+1]);
	}

	return livingCells;
} // loadData

/*
 * Fills the local board with live cells based on rank's column range
 * Each rank handles columns from offset to offset + local_cols
 */
void fillboard(int *liveCells, int rank, int size, int global_COLSIZ)
{
	int i, row, col;
	int local_cols = global_COLSIZ / size;
	int col_offset = local_cols * rank;

	for (i = 0; i < numOfLivingCells * 2; i+=2){
		int global_row = liveCells[i];
		int global_col = liveCells[i+1];

		// Check if this cell belongs to this rank's column range
		if (global_col >= col_offset && global_col < col_offset + local_cols) {
			// Map to local coordinates (with +1 offset for halo)
			row = global_row + 1;
			col = (global_col - col_offset) + 1; 
			board[0][row][col] = 1;
		}
	}
}