#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// I swear there is a native c function for this lol
void replace(int index, char *str, char *replace) {
    int i = 0;
    while (replace[i] != '\0') {
        str[index + i] = replace[i];
        i++;
    }
    // strcpy(str + index, replace);
}

int addPixel(int index, char *pixels) {
    // RED
    pixels[index] = '\x00';
    pixels[index + 1] = '\x00';
    pixels[index + 2] = '\xFF';
    return index + 3;
}

int main() {
    FILE *image = fopen("image.bmp", "w");

    const int BFH_SIZE = 14;
    const int DIB_SIZE = 12;

    // TODO: Make this static / define
    const short WIDTH = 2;
    const short HEIGHT = 2;
    const int PADDING_BYTES = (WIDTH * HEIGHT);
    const int PIXEL_SIZE = (WIDTH * HEIGHT * 3) + PADDING_BYTES;

    const long int SIZE = BFH_SIZE + DIB_SIZE + PIXEL_SIZE;

    // Bitmap file header
    char bfh[BFH_SIZE] = {};
    char *p = bfh;
    replace(0, bfh, "BM");
    p[2] = SIZE;
    // replace(6, bfh, "\x00\x00\x00\x00"); // Unused
    // p[4] = "\x00\x00\x00\x00";           // Unused
    p[10] = (short)(BFH_SIZE + DIB_SIZE);

    fwrite(bfh, sizeof(char), BFH_SIZE, image);

    // DIB header
    short PLANES = 1;

    char dib[DIB_SIZE] = {};
    dib[0] = DIB_SIZE;
    dib[4] = WIDTH;
    dib[6] = HEIGHT;
    dib[8] = PLANES;
    dib[10] = (short)24;

    fwrite(dib, sizeof(char), DIB_SIZE, image);

    // Pixel array
    char pixels[PIXEL_SIZE] = {};

    addPixel(0, pixels);
    addPixel(3, pixels);
    // padding
    addPixel(6 + 2, pixels);
    addPixel(9 + 2, pixels);
    // padding
    fwrite(pixels, sizeof(char), PIXEL_SIZE, image);

    fclose(image);
    printf("Done\n");
}