#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// #define WIDTH 64
// #define HEIGHT 64
#define WIDTH 128
#define HEIGHT WIDTH

#define OFFSET_X 2
#define OFFSET_Y 1

#define MAX_ITER 1000

// I swear there is a native c function for this lol
void replace(int index, char *str, char *replace) {
    int i = 0;
    while (replace[i] != '\0') {
        str[index + i] = replace[i];
        i++;
    }
    // strcpy(str + index, replace);
}

void shortToChar(char *buf, short number) {
    buf[0] = number >> 8;
    buf[1] = number & 0xFF;
}

void printGrid(double *grid) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        printf("%f, ", grid[i]);

        if (i % HEIGHT == 0) {
            printf("\n");
        }
    }
}

void computeColour(char *colour, char *palette, double cell) {
    // Assuming cell is iter
    int i = (int)cell * 3 * 5;
    colour[0] = palette[i];
    colour[1] = palette[i + 1];
    colour[2] = palette[i + 2];
}

void computePalette(char *palette) {
    // max iter
    for (int i = 0; i < MAX_ITER; i += 3) {
        int v = pow(i, 2.25);
        palette[i] = (v / MAX_ITER) * 128;
        palette[i + 1] = ((MAX_ITER - v) / MAX_ITER) * 255;
        palette[i + 2] = 255;
    }
}

int saveToImage(char *filename, double *grid) {
    FILE *image = fopen(filename, "w");

    const int BFH_SIZE = 14;
    const int DIB_SIZE = 12;

    // TODO: Make this static / define
    // const int PADDING_BYTES = HEIGHT * 2;
    const int PIXEL_SIZE = (WIDTH * HEIGHT * 3);

    const long int SIZE = BFH_SIZE + DIB_SIZE + PIXEL_SIZE;

    // Bitmap file header
    char bfh[BFH_SIZE] = {};
    char *p = bfh;
    replace(0, bfh, "BM");
    p[2] = SIZE; // this will overflow (since its int -> char)
    p[10] = (short)(BFH_SIZE + DIB_SIZE);

    fwrite(bfh, sizeof(char), BFH_SIZE, image);

    // DIB header
    short PLANES = 1;

    short dib[5] = {};
    dib[0] = DIB_SIZE;
    dib[2] = WIDTH;
    dib[3] = HEIGHT;
    dib[4] = PLANES;
    dib[5] = (short)24;

    fwrite(dib, sizeof(short), DIB_SIZE, image);

    // Pixel array
    char pixels[PIXEL_SIZE] = {};

    // Palette
    char palette[MAX_ITER * 3] = {};
    computePalette(palette);

    // Unsure why I need to - 5 to prevent weird color issues
    int i = PIXEL_SIZE - 5;
    // printGrid(grid);
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int gridIndex = y + (x * HEIGHT);
            char c0[3];
            computeColour(c0, palette, grid[gridIndex]);

            pixels[i] = c0[0];
            pixels[i - 1] = c0[1];
            pixels[i - 2] = c0[2];
            i -= 3;
        }

        // Padding
        while ((PIXEL_SIZE - i) % 4 != 0) {
            pixels[i] = '\0'; // padding
            i--;
        }
    }

    fwrite(pixels, sizeof(char), PIXEL_SIZE, image);
    fclose(image);
    return 0;
}

void computeGrid(double *grid) {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            // Unsure why I have to do WIDTH - 1 to prevent stack smashing
            int wi = (WIDTH - 1) - i;
            int index = j + (wi * WIDTH);
            double sX = (double)i / WIDTH;
            double sY = (double)j / HEIGHT;
            double complex c = ((sX * 3.47) - OFFSET_X) + (((sY * 2.24) - OFFSET_Y) * I);
            double complex z = 0 + 0 * I;
            int iter = 0;
            while (cabs(z) <= 2 && iter < MAX_ITER) {
                z = cpow(z, 2) + c;
                iter++;
            }

            grid[index] = (double)iter;
        }
    }
}

int main() {
    double grid[WIDTH * HEIGHT] = {};
    computeGrid(grid);
    saveToImage("image.bmp", grid);
    printf("Done\n");
}