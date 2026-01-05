// The Game Board - with two boards: current and next-gen
//goal: board[switch][ROWSIZ][COLSIZ]; 2 boards for swapping generations

#include <stdio.h>
#include <stdlib.h>

extern char **board[2];
extern int ROWSIZ, COLSIZ;

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
displayBoard(int type, char **bp)
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
	for (col=colfirst; col < colsize; col++) {
	    printf("-");
	}
	printf("\n");

	for (row=rowfirst; row < rowsize; row++) {
	    for (col=colfirst; col < colsize; col++) {
	      printf ("%c", bp[row][col]?'@':'.');  // live=='@'
	    }
	    printf ("|\n");		// add an edge on the RHS
	}
	
    } else {				// i.e., type==0
	// never report on the halo
        printf ("%d %d\n", ROWSIZ-2, COLSIZ-2);
	for (row=1; row < ROWSIZ-1; row++) {
	    for (col=1; col < COLSIZ-1; col++) {
	      if(bp[row][col]) {
		  printf ("%d %d\n", row, col);
	      }
	    }
	}
    }

} // displayBoard

/*
 *  * read inputfile - two integers per line; first line is size of board
 *   */
void
loadlife()
{
    int x,y;
    /* first, read in the board size */
    scanf("%d %d\n", &ROWSIZ, &COLSIZ);
    // add space for the buffers (+2)
    ROWSIZ += 2;
    COLSIZ += 2;
    /* now allocate it */
    initBoard(ROWSIZ, COLSIZ);

    /* now read in the existing live cells into board [0] */
    while (scanf("%d %d", &x, &y) != EOF) {
        board[0][x][y] = 1;
    }

} // loadlife

