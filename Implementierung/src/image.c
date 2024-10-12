#define _XOPEN_SOURCE 700
#include "image.h"
#include <stdio.h>
#include <sys/stat.h>

#define READ_ERROR "Error reading image: not a valid netpbm format file!\nOnly 24bpp PPM is allowed as input format!"
// function that prints an error message and exits the program
int error(const char* message, FILE* file, int read)
{
    fprintf(stderr, "%s\n", message);
    if (file)
        fclose(file);
    if (read)
        exit(EXIT_FAILURE);
    // when writing, need to free memory allocated on the heap in main before exiting
    return EXIT_FAILURE;
}

// function that skips whitespace and comments in the netpbm file
void skip(FILE* file)
{
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '#') {
            while ((c = fgetc(file)) != EOF && c != '\n' && c != '\r')
                ;
        } else if (c != '\n' && c != ' ' && c != '\r' && c != '\t' && c != '\v' && c != '\f') {
            ungetc(c, file);
            return;
        }
    }
    error("Unexpected end of file!", file, 1);
}

void read_image(const char* path, struct Netpbm* image)
{
    FILE* input_image = fopen(path, "rb");
    if (!input_image)
        error("Could not open input file!", NULL, 1);
    // check if the file is a valid 24bpp P6 PPM image
    struct stat statbuf;
    if (fstat(fileno(input_image), &statbuf) < 0 || !S_ISREG(statbuf.st_mode) || statbuf.st_size == 0)
        error("Invalid input file!", input_image, 1);
    skip(input_image);
    if (fread(image->magicNumber, sizeof(char), 2, input_image) != 2 || image->magicNumber[0] != 'P' || image->magicNumber[1] != '6')
        error(READ_ERROR, input_image, 1);
    image->magicNumber[2] = '\0';
    skip(input_image);
    if (fscanf(input_image, "%zu", &image->width) <= 0 || image->width == 0)
        error(READ_ERROR, input_image, 1);
    skip(input_image);
    if (fscanf(input_image, "%zu", &image->height) <= 0 || image->height == 0)
        error(READ_ERROR, input_image, 1);
    skip(input_image);
    if (fscanf(input_image, "%hu", &image->maxValue) <= 0 || image->maxValue > 255)
        error(READ_ERROR, input_image, 1);
    skip(input_image);
    // read the pixels into an array
    size_t array_size = image->width * image->height * 3;
    image->pixels = malloc(array_size);
    if (!image->pixels)
        error("Could not allocate memory for image pixels!", input_image, 1);
    if (fread(image->pixels, sizeof(uint8_t), array_size, input_image) != array_size) {
        free(image->pixels);
        error(READ_ERROR, input_image, 1);
    }

    fclose(input_image);
}

int write_image(const struct Netpbm* image, const char* outputPath)
{
    FILE* output_image = fopen(outputPath, "wb");
    if (!output_image)
        return error("Could not open/create output file!", NULL, 0);

    fprintf(output_image, "%s\n%zu %zu\n%u\n", image->magicNumber, image->width, image->height, image->maxValue);
    size_t array_size = image->width * image->height;
    if (fwrite(image->pixels, sizeof(uint8_t), array_size, output_image) != array_size)
        return error("Could not write image to file!", output_image, 0);

    fclose(output_image);
    return EXIT_SUCCESS;
}