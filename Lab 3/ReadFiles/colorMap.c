#include <stdio.h>
#include <stdlib.h>
#include "colorMap.h"
#include "readData.h"

/*
 * colorMap - read up a color map
 *
 * returns a pointer to the color map structs
 *
 */

struct colorMap *
loadColorMap(char *cmfile, int size)
{
    int aline[4], nrd, i;
    struct colorMap *cmap, *p;
    FILE *fp;

    cmap = calloc(sizeof(struct colorMap), size);

    fp = fopen(cmfile, "r");

    for(i=0, p=cmap; (i < size);  i++, p++) {
	nrd = readData(fp, aline, 4);
	if (nrd == 0) break;
	p->val = aline[0];
	p->red = aline[1];
	p->green = aline[2];
	p->blue = aline[3];
    }
    return cmap;

} // loadColorMap
