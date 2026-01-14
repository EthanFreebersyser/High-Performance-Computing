#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Converts Game of Life output format to colorIt data format
 * Input: rows cols numLiveCells followed by row/col pairs
 * Output: grid of values (0 for dead, 1 for alive)
 */
int main(int argc, char **argv) {
    int rows, cols, numLiveCells;
    int i, j;
    int row, col;
    int *grid;

    FILE *fp = fopen(argv[1], "r");

    // Read rows cols numLiveCells
    fscanf(fp, "%d %d %d\n", &rows, &cols, &numLiveCells);

    // Allocate grid (initialized to 0 = dead)
    grid = calloc((size_t)(rows * cols), sizeof(int));

    // Read live cell coordinates and mark them in grid
    for (i = 0; i < numLiveCells; i++) {
        fscanf(fp, "%d %d\n", &row, &col);

        // Validate coordinates
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            continue;
        }
        
        // Mark cell as alive (1)
        grid[row * cols + col] = 1;
    }

    fclose(fp);

    // Output grid for colorIt
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%d\n", grid[i * cols + j]);
        }
    }

    free(grid);
    return 0;
}