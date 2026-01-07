#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"


#include "../ReadFiles/colorMap.h"

#include "../ReadFiles/readData.h"

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
	fprintf(stderr, "usage: %s imageheight\n", argv[0]);
	return 2;
    }
    width = atoi(argv[1]);	
    height = atoi(argv[2]);
    img_dims = width*height;		// # of pixels
    rowsize = width*channels;		// bytes per row (3 per pixel)
    ra_size = height*rowsize;           // total # of bytes needed
    img = malloc(ra_size);

    char *colorMapFile = argv[3];
    int cMapSize;

    if (strcmp(colorMapFile, "mystery1.data") == 0){
        cMapSize = 250000;
    } else if (strcmp(colorMapFile, "mystery2.data") == 0){
        cMapSize = 458724;
    } else if (strcmp(colorMapFile, "mystery3.data") == 0){
        cMapSize = 1701632;
    } else if (strcmp(colorMapFile, "mystery4.data") == 0){
        cMapSize = 250000;
    } else if (strcmp(colorMapFile, "mystery5.data") == 0){
        cMapSize = 250500;
    } else {
        cMapSize = 0;
    }
   
    struct colorMap *colorMap = loadColorMap(colorMapFile, cMapSize);

    FILE *fileptr = fopen(argv[4], "r");

    int *pixels = (int *)malloc(sizeof(int) * (size_t)img_dims);

    int numread = readData(fileptr, pixels, (int)img_dims);

    int maxVal = 0;
    for (i = 0; i < cMapSize; i++) {
        if (colorMap[i].val > maxVal){
            maxVal = colorMap[i].val;
        }
    }

    int *lookUp = malloc(sizeof(int) * (size_t)(maxVal + 1));
    
    int v;
    for (v = 0; v <= maxVal; v++) lookUp[v] = -1;

    for (i = 0; i < cMapSize; i++){
        int value = colorMap[i].val;
        if (value >= 0 && value <= maxVal) lookUp[i] = v;
    }

    for(i=0; i < img_dims; i++) {
        int value = pixels[i];

        char r = 0, g = 0, b = 0;

        if (value >= 0 && value <= maxVal){
            int idx = lookUp[value];
            if (idx >= 0){
                r = colorMap[idx].red;
                g = colorMap[idx].green;
                b = colorMap[idx].blue;
            }
        }

        img[3 * i + 0] = r;
        img[3 * i + 1] = g;
        img[3 * i + 2] = b;	   
    }

    /*
     * Write a PNG image:
     */
    stbi_write_png("sample.png", width, height, channels, img, rowsize);

    free(img);
    free(pixels);
    free(colorMap);


    return 0;
} // main
