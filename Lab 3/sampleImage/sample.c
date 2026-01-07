#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

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
    width = height = atoi(argv[1]);	// this is a square image
    img_dims = width*height;		// # of pixels
    rowsize = width*channels;		// bytes per row (3 per pixel)
    ra_size = height*rowsize;           // total # of bytes needed
    img = malloc(ra_size);

    for(p=img, i=0; i <= img_dims; i++, p += channels) {
	    // just to make some interesting colors:
	    // work our way up to 255 then back down
	    if (i/width  <= 255) {
		*p = ((i/width)%256);   //R
	    } else {
		*p = 255-((i/width)%256);   //R
	    }
	    *(p+1) = *p;    //G
	    *(p+2) = 100;   //B

    } // next pixel

    /*
     * Write a PNG image:
     */
    stbi_write_png("sample.png", width, height, channels, img, rowsize);

    free(img);

    return 0;
} // main
