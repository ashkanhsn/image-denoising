#define _POSIX_C_SOURCE 200809L
#include "performance_tests.h"
#include "../src/combine.h"
#include "../src/convolution.h"
#include "../src/denoise.h"
#include "../src/grayscale.h"
#include "../src/image.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// path to image used for performance tests:
char* path = "samples/noisylowres.ppm";
uint16_t iterations = 100;

struct Netpbm image;
uint8_t* grayscale_image;
uint8_t* rgb_image;
uint8_t* blurred;
uint8_t* result;
uint8_t* laplaced;
uint16_t* padded_image;
uint16_t* padded_laplace;
uint16_t* padded_blur;
size_t width;
size_t height;
size_t padded_width;
size_t padded_height;
struct timespec start, end;
typedef void (*GrayscaleFunc)(const unsigned char*, long unsigned int, long unsigned int, float, float, float, unsigned char*);

#define timer(function_call, time_taken_name)                                                   \
    {                                                                                           \
        clock_gettime(CLOCK_MONOTONIC, &start);                                                 \
        for (uint64_t i = 0; i < iterations; i++) {                                             \
            (function_call);                                                                    \
        }                                                                                       \
        clock_gettime(CLOCK_MONOTONIC, &end);                                                   \
        (time_taken_name) = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9; \
    }

int setup()
{
    read_image(path, &image);
    height = image.height;
    width = image.width;
    rgb_image = image.pixels;
    padded_height = height + 2;
    padded_width = width + 2;

    grayscale_image = malloc(width * height * sizeof(uint8_t));
    blurred = malloc(width * height * sizeof(uint8_t));
    laplaced = malloc(width * height * sizeof(uint8_t));
    result = malloc(width * height * sizeof(uint8_t));
    padded_image = malloc(padded_width * padded_height * sizeof(uint16_t));
    padded_laplace = malloc(padded_width * padded_height * sizeof(uint16_t));
    padded_blur = malloc(padded_width * padded_height * sizeof(uint16_t));

    if (!grayscale_image || !blurred || !result || !laplaced || !padded_laplace || !padded_blur || !padded_image)
        return 1;
    return 0;
}

double time_grayscale_implementation(GrayscaleFunc func, const char* description)
{
    double time_taken;
    timer(func(rgb_image, width, height, 0.2126, 0.7152, 0.0722, grayscale_image), time_taken);
    printf("Time taken for %s: %f seconds\n", description, time_taken);
    return time_taken;
}

int test_grayscale_performance()
{
    if (!rgb_image || !grayscale_image) {
        if (setup())
            return 1;
    }
    double time_taken_accurate = time_grayscale_implementation((GrayscaleFunc)grayscale, "Grayscale Accurate");
    double time_taken_integer = time_grayscale_implementation((GrayscaleFunc)grayscale_integer, "Grayscale Integer");
    double time_taken_simd = time_grayscale_implementation((GrayscaleFunc)grayscale_simd, "Grayscale SIMD");

    printf("Time for Grayscale Integer as percentage of accurate: %f\n", time_taken_integer / time_taken_accurate * 100);
    printf("Time for Grayscale SIMD as percentage of accurate: %f\n\n", time_taken_simd / time_taken_accurate * 100);
    return 0;
}

int test_convolution_performance()
{
    double time_convolution_accurate, time_convolution_blur;
    timer(convolution(grayscale_image, width, height, laplaced, laplace_kernel, 1), time_convolution_accurate);
    timer(convolution(grayscale_image, width, height, blurred, blur_kernel, 0), time_convolution_blur);
    time_convolution_accurate += time_convolution_blur;
    printf("Time taken for %s: %f seconds\n", "Convolution Accurate", time_convolution_accurate);

    double time_convolution_integer;
    timer(convolution_1pass(grayscale_image, width, height, laplaced, blurred), time_convolution_integer);
    printf("Time taken for %s: %f seconds\n", "Convolution Integer", time_convolution_integer);

    double time_padding, time_convolution_simd;
    timer(pad_image_simd(rgb_image, width, height, padded_width, padded_image), time_padding);
    // printf("Time taken for %s: %f seconds\n", "Padding 1", time_padding);
    timer(convolution_simd(padded_image, padded_width, padded_height, padded_laplace, padded_blur), time_convolution_simd);
    time_convolution_simd += time_padding;
    printf("Time taken for %s: %f seconds\n", "Convolution Simd", time_convolution_simd);

    printf("Time for Convolution Integer as percentage of accurate: %f\n", time_convolution_integer / time_convolution_accurate * 100);
    printf("Time for Convolution SIMD as percentage of accurate: %f\n\n", time_convolution_simd / time_convolution_accurate * 100);
    return 0;
}

int test_combine_performance()
{
    double time_taken_accurate;
    timer(combine(grayscale_image, laplaced, blurred, width, height, result, 1), time_taken_accurate);
    printf("Time taken for %s: %f seconds\n", "Combine Accurate", time_taken_accurate);

    double time_taken_integer;
    timer(combine(grayscale_image, laplaced, blurred, width, height, result, 0), time_taken_integer);
    printf("Time taken for %s: %f seconds\n", "Combine Integer", time_taken_integer);

    double time_taken_simd;
    timer(combine_simd(grayscale_image, padded_laplace, padded_blur, width, height, padded_width, result), time_taken_simd);
    printf("Time taken for %s: %f seconds\n", "Combine Simd", time_taken_simd);

    printf("Time for Combine Integer as percentage of accurate: %f\n", time_taken_integer / time_taken_accurate * 100);
    printf("Time for Combine SIMD as percentage of accurate: %f\n\n", time_taken_simd / time_taken_accurate * 100);
    return 0;
}

int test_denoise_performance()
{
    double time_taken_accurate;
    timer(denoise(rgb_image, width, height, 0.2126, 0.7152, 0.0722, laplaced, blurred, result), time_taken_accurate);
    printf("Time taken for %s: %f seconds\n", "Denoise Accurate", time_taken_accurate);

    double time_taken_integer;
    timer(denoise_integer(rgb_image, width, height, 0.2126, 0.7152, 0.0722, laplaced, blurred, result), time_taken_integer);
    printf("Time taken for %s: %f seconds\n", "Denoise Integer", time_taken_integer);

    double time_taken_simd;
    timer(denoise_simd(rgb_image, width, height, 0.2126, 0.7152, 0.0722, padded_image, padded_laplace, padded_blur, result), time_taken_simd);
    printf("Time taken for %s: %f seconds\n", "Denoise SIMD", time_taken_simd);

    printf("Time for Denoise Integer as percentage of accurate: %f\n", time_taken_integer / time_taken_accurate * 100);
    printf("Time for Denoise SIMD as percentage of accurate: %f\n\n", time_taken_simd / time_taken_accurate * 100);
    return 0;
}

int run_all_perf_tests()
{
    printf("\nTesting performance with %s at %i iterations...\n\n", path, iterations);
    int a = 0;
    if (setup() || test_grayscale_performance() || test_convolution_performance() || test_combine_performance() || test_denoise_performance())
        a = 1;
    free(grayscale_image);
    free(blurred);
    free(result);
    free(laplaced);
    free(padded_laplace);
    free(padded_blur);
    free(padded_image);
    free(image.pixels);
    return a;
}