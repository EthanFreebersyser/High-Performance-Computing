/*
 * Conway's game of Life
 *
 */
#include <stdio.h>
#include <stdlib.h>

char **board[2];
int XSIZ, YSIZ;

/*
 *  Fill the edges of the board with data to make a
 *  periodic buffer ("wraparound")
 */
void
periodicBuffer(char **theboard)		// theboard - a 2D array
{
    // TODO:  write code for this

} // periodicBuffer

/*
 *  newLife - compute the "life" for cell board[i][j]
 *  		using data from oldboard and putting the
 *  		result (the next generation) into the newboard
 */
void
newLife(char **newboard, char **oldboard, int r, int c)
{
    // TODO:  write code for this
    // This code is just a place holder and should be replaced:
    newboard[r][c] = oldboard[r][c];

} // newLife

int
main(int argc, char **argv)
{
    int i,j;
    int gen, genOut, period;

    if (argc > 1) {
	genOut = atoi(argv[1]);
	period = atoi(argv[2]);
    } else {
	printf ("usage: life generations output-frequency < infile\n");
	exit(-1);
    }

    // read inputfile and init array
    loadlife();
    fprintf(stderr, "load complete\n");

    for(gen=0; gen < genOut; gen++) {

	if (period > 0 && gen%period == 0) {
	    displayBoard(period, board[gen&1]);
	    system("sleep 0.05");	// TODO: delete for speed
	 }

	 // TODO: periodicBuffer(board[(gen & 1)]);

	for (i=1; i < XSIZ-1; i++) {
	    for (j=1; j < YSIZ-1; j++) {
		if (gen & 1) {
		    newLife(board[0], board[1], i, j);
		} else {
		    newLife(board[1], board[0], i, j);
		}
	    } // next j
	} // next i

    } // next gen

    if (period == 0 || gen%period != 0) {
	displayBoard(period, board[1-(gen&1)]);  // show last board updated
	if (period > 0) {
	    printf("gen:%d\n", gen);
	}
    }

} // main

