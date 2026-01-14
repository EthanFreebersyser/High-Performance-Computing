#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include "../ReadFiles/colorMap.h"

#include "../ReadFiles/readData.h"

int countLines(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    int count = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') count++;
    }
    fclose(fp);
    return count;
}

int findColor(int value, struct colorMap *cmap, int cmapSize){
    for (int i = 0; i < cmapSize; i++) {
        if (value <= cmap[i].val) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char **argv) {
    int i;
    int width, height;
    unsigned char *img, *p;
    long ra_size;
    long img_dims;
    int channels = 3;		// R G B
    int rowsize;
    unsigned char val;
    
    if (argc < 2) {
	    fprintf(stderr, "usage: %s width height colorMapFile dataFile\n", argv[0]);
	    return 2;
    }
    width = atoi(argv[1]);	
    height = atoi(argv[2]);
    img_dims = width*height;		// # of pixels
    rowsize = width*channels;		// bytes per row (3 per pixel)
    ra_size = height*rowsize;       // total # of bytes needed
    img = malloc(ra_size);

    char *colorMapFile = argv[3];
    char *dataFile = argv[4];

    //get the size of the color map
    int cmapSize = countLines(colorMapFile);

    struct colorMap *cmap = loadColorMap(colorMapFile, cmapSize);

    FILE *fileptr = fopen(dataFile, "r");

    int *pixels = (int *)malloc(sizeof(int) * (size_t)img_dims);

    int numread = readData(fileptr, pixels, (int)img_dims);

    for(i = 0; i < img_dims; i++) {
        int value = pixels[i];

        char r = 0, g = 0, b = 0;

        int idx = findColor(value, cmap, cmapSize);
        if (idx >=0){
            r = cmap[idx].red;
            g = cmap[idx].green;
            b = cmap[idx].blue;
        }

        img[3 * i + 0] = r;
        img[3 * i + 1] = g;
        img[3 * i + 2] = b;	   
    }

    /*
     * Write a PNG image:
     */
    stbi_write_png("colorIt.png", width, height, channels, img, rowsize);

    free(img);
    free(pixels);
    free(cmap);
    fclose(fileptr);

    return 0;
} // main
