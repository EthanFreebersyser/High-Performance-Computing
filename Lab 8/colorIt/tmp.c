#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include "ReadFiles/colorMap.h"

#include "ReadFiles/readData.h"

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
        if (value == cmap[i].val) {
            return i;
        }
    }
    return -1;
}

// Comparison function for sorting filenames
int compareFiles(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main(int argc, char **argv) {
    int i;
    int width, height;
    unsigned char *img;
    long ra_size;
    long img_dims;
    int channels = 3;		// R G B
    int rowsize;
    
    if (argc < 5) {
	    fprintf(stderr, "usage: %s width height colorMapFile inputDir outputDir\n", argv[0]);
	    return 2;
    }
    width = atoi(argv[1]);	
    height = atoi(argv[2]);
    img_dims = width * height;		// # of pixels
    rowsize = width * channels;		// bytes per row (3 per pixel)
    ra_size = height * rowsize;     // total # of bytes needed
    img = malloc(ra_size);

    char *colorMapFile = argv[3];
    char *inputDir = argv[4];
    char *outputDir = argv[5];

    // Create output directory if it doesn't exist
    mkdir(outputDir, 0755);

    // Get the size of the color map
    int cmapSize = countLines(colorMapFile);
    struct colorMap *cmap = loadColorMap(colorMapFile, cmapSize);

    // Allocate pixel buffer (reused for each file)
    int *pixels = (int *)malloc(sizeof(int) * (size_t)img_dims);

    // Open the input directory
    DIR *dir = opendir(inputDir);
    if (!dir) {
        fprintf(stderr, "Error: Could not open input directory %s\n", inputDir);
        return 1;
    }

    // First pass: count .data files
    struct dirent *entry;
    int fileCount = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".data") != NULL) {
            fileCount++;
        }
    }
    rewinddir(dir);

    // Collect filenames
    char **filenames = malloc(sizeof(char *) * fileCount);
    int idx = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".data") != NULL) {
            filenames[idx] = strdup(entry->d_name);
            idx++;
        }
    }
    closedir(dir);

    // Sort filenames so output is in order
    qsort(filenames, fileCount, sizeof(char *), compareFiles);

    // Process each file
    for (int f = 0; f < fileCount; f++) {
        char inputPath[512];
        char outputPath[512];

        snprintf(inputPath, sizeof(inputPath), "%s/%s", inputDir, filenames[f]);
        snprintf(outputPath, sizeof(outputPath), "%s/%d.png", outputDir, f);

        FILE *fileptr = fopen(inputPath, "r");
        if (!fileptr) {
            fprintf(stderr, "Warning: Could not open %s\n", inputPath);
            continue;
        }

        int numread = readData(fileptr, pixels, (int)img_dims);
        fclose(fileptr);

        // Convert data to image
        for (i = 0; i < img_dims; i++) {
            int value = pixels[i];

            unsigned char r = 0, g = 0, b = 0;

            int colorIdx = findColor(value, cmap, cmapSize);
            if (colorIdx >= 0) {
                r = cmap[colorIdx].red;
                g = cmap[colorIdx].green;
                b = cmap[colorIdx].blue;
            }

            img[3 * i + 0] = r;
            img[3 * i + 1] = g;
            img[3 * i + 2] = b;	   
        }

        // Write PNG image
        stbi_write_png(outputPath, width, height, channels, img, rowsize);

        printf("Processed %s -> %s\n", filenames[f], outputPath);

        free(filenames[f]);
    }

    free(filenames);
    free(img);
    free(pixels);
    free(cmap);

    printf("Done! Processed %d files.\n", fileCount);

    return 0;
} // main