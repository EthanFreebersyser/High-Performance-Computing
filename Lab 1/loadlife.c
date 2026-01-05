// The Game Board - with two boards: current and next-gen
//goal: board[switch][XSIZ][YSIZ]; 2 boards for swapping generations

#include <stdio.h>
#include <stdlib.h>

extern char **board[2];
extern int XSIZ, YSIZ;

/*
 * create both boards: [0] and [1]
 */
void initBoard(int xsize, int ysize)
{
    int b, i;

    for(b=0;b<=1;b++) {
        board[b] = calloc((size_t)xsize, sizeof(char *));
        for (i=0; i < xsize; i++) {
            board[b][i] = calloc((size_t)ysize, sizeof(char));
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
    int i,j;
    int xfirst, yfirst;
    int xsize, ysize;

    if (type) {
	if (type >= 1) {	// just the board
	    xfirst=1;
	    xsize=XSIZ-1;
	    yfirst=1;
	    ysize=YSIZ-1;
	} else {		// include the halo (debugging)
	    xfirst=0;
	    xsize=XSIZ;
	    yfirst=0;
	    ysize=YSIZ;
	}
	// add a delimiter between successive boards (at the top)
	for (i=xfirst; i < xsize; i++) {
	    printf("-");
	}
	printf("\n");

	for (j=yfirst; j < ysize; j++) {
	    for (i=xfirst; i < xsize; i++) {
	      printf ("%c", bp[i][j]?'@':' ');  // live=='@'
	    }
	    printf ("|\n");		// add an edge on the RHS
	}
	
    } else {				// i.e., type==0
	// never report on the halo
        printf ("%d %d\n", XSIZ-2, YSIZ-2);
	for (j=1; j < YSIZ-1; j++) {
	    for (i=1; i < XSIZ-1; i++) {
	      if(bp[i][j]) {
		  printf ("%d %d\n", i, j);
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
    scanf("%d %d\n", &XSIZ, &YSIZ);
    // add space for the buffers (+2)
    XSIZ += 2;
    YSIZ += 2;
    /* now allocate it */
    initBoard(XSIZ, YSIZ);

    /* now read in the existing live cells into board [0] */
    while (scanf("%d %d", &x, &y) != EOF) {
        board[0][x][y] = 1;
    }

} // loadlife

