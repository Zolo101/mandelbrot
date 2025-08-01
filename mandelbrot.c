#include "mandelbrot.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// const short WIDTH = 1280;
// const short HEIGHT = 720;

const short WIDTH = 1920;
const short HEIGHT = 1280;

const size_t MAX_ITER = 100;

// const double OFFSET_X = 2.5;
// const double OFFSET_Y = 1.1;
const double OFFSET_X = 0.228155493653962;
const double OFFSET_Y = 1.115142508039937;
const double SCALE = 64.0;

const int BFH_SIZE = 14;
const int DIB_SIZE = 12;

const int PIXEL_SIZE = (WIDTH * HEIGHT * 3);

const short HEADER_SIZE = BFH_SIZE + DIB_SIZE;
const long SIZE = BFH_SIZE + DIB_SIZE + PIXEL_SIZE;

void printGrid(double *grid) {
    for (size_t i = 0; i < WIDTH * HEIGHT; i++) {
        printf("%f, ", grid[i]);

        if (i % HEIGHT == 0) {
            printf("\n");
        }
    }
}

void computePalette(char palette[MAX_ITER][3]) {
    for (size_t i = 0; i < MAX_ITER; i++) {
        size_t x = pow(i, 2);
        size_t v = 128 - x;

        char colour[3] = {v, 1 - v, 1 - (v / 2)};
        memcpy(palette[i], colour, 3);
    }
}

int saveToImage(char *filename, double *grid) {
    FILE *image = fopen(filename, "wb");

    // Bitmap file header
    fwrite("BM", sizeof(char), 2, image);
    fwrite(&SIZE, sizeof(int), 1, image);
    fwrite("\x00\x00\x00\x00", sizeof(char), 4, image);
    fwrite(&HEADER_SIZE, sizeof(int), 1, image);

    // DIB header
    short PLANES = 1;
    short DEPTH = 24;
    short W = WIDTH;
    short H = HEIGHT;

    fwrite(&DIB_SIZE, sizeof(int), 1, image);
    fwrite(&W, sizeof(short), 1, image);
    fwrite(&H, sizeof(short), 1, image);
    fwrite(&PLANES, sizeof(short), 1, image); // PLANES
    fwrite(&DEPTH, sizeof(short), 1, image);

    // Pixel array
    char pixels[PIXEL_SIZE] = {};

    // Palette
    char palette[MAX_ITER][3] = {};
    computePalette(palette);

    for (size_t v = 0; v < WIDTH * HEIGHT; v++) {
        int value = (int)grid[v] - 1; // - 1 for indexing
        size_t b = v * 3;
        memcpy(pixels + b, palette[value], sizeof(char) * 3);
    }

    fwrite(pixels, sizeof(char), PIXEL_SIZE, image);
    fclose(image);
    return 0;
}

void computeGrid(double *grid) {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            size_t index = x + (y * WIDTH);
            double sX = (double)x / WIDTH / SCALE;
            double sY = (double)y / HEIGHT / SCALE;
            double complex c = ((sX * 3.47) - OFFSET_X) + (((sY * 2.24) - OFFSET_Y) * I);
            double complex z = 0 + 0 * I;
            size_t iter = 0;
            while (cabs(z) <= 2 && iter < MAX_ITER) {
                z = (z * z) + c;
                iter++;
            }

            // printf("coord (%d, %d)\n", x, y);
            // printf("other (%d, %d)\n\n", x, index);
            grid[index] = (double)iter;
        }
    }
}

int main() {
    double *grid = calloc(WIDTH * HEIGHT, sizeof(double));
    if (grid == NULL) {
        printf("Failed to allocate grid memory!\n");
        return 1;
    }

    computeGrid(grid);
    saveToImage("image.bmp", grid);

    free(grid);
    printf("Done\n");
    return 0;
}