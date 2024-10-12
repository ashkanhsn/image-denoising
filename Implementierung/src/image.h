#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdlib.h>

struct Netpbm {
    char magicNumber[3];
    uint16_t maxValue;
    size_t width;
    size_t height;
    uint8_t* pixels;
};

// Read a PPM image from a file, exits the program on error
void read_image(const char* imagePath, struct Netpbm* image);

// Write a PGM image to a file, returns 0 on success
int write_image(const struct Netpbm* image, const char* outputPath);

#endif // IMAGE_H
