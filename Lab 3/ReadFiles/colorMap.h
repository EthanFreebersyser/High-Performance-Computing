#ifndef COLORMAP
#define COLORMAP

struct colorMap {
    int val;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

struct colorMap *loadColorMap(char *cmfile, int size);
#endif
