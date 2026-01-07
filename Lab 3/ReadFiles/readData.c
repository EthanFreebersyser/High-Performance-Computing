#include <stdio.h>

/*
 * readData - read a line from a datafile
 *     caller supplies a file ptr, an array of ints, and a size
 *	the function returns the # of ints read
 *
 * example use:
 *    fp = fopen("mydata.file", "r");
 *    numread = readData(fp, ra, nints);
 * where ra is an array or some malloc'd storage
 *   and nints holds the # of ints that you want to read
 */

int
readData(FILE *fptr, int *bufr, int count)
{
    int i, val;
    for (i=0; ((i < count) && (fscanf(fptr, " %d", &val) != EOF)) ; i++) {
	*bufr = val;
	bufr++;
    }
    return i;

} // readData
