#define _POSIX_C_SOURCE 200809L
#include "../src/denoise.h"
#include "../src/image.h"
#include "../tests/functional_tests.h"
#include "../tests/performance_tests.h"
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define IS_DIGIT(c) ((c >= '0' && c <= '9') ? 1 : 0)

struct option long_options[] = {
    { "help", no_argument, NULL, 'h' },
    { "coeffs", required_argument, NULL, 'c' },
    { NULL, 0, NULL, 0 }
};

long parseX(char* optarg, char* option)
{
    errno = 0;
    if (optarg == NULL) {
        fprintf(stderr, "Could not parse argument for option %s!\n", option);
        return -1;
    }
    char* endptr;
    long x = strtol(optarg, &endptr, 10);
    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Could not parse argument for option %s!\n", option);
        printf("For more information, run the program with the --help option.\n");
        return -1;
    }
    if (option[1] == 'V' && (x < 0 || x > 2)) {
        fprintf(stderr, "Argument for option %s must be 0 or 1 or 2!\n", option);
        printf("For more information, run the program with the --help option.\n");
        return -1;
    }
    if (option[1] == 'B' && x < 1) {
        fprintf(stderr, "Argument for option %s must be greater than 0!\n", option);
        printf("For more information, run the program with the --help option.\n");
        return -1;
    }
    return x;
}

void cleanup_end(int status, int argc, ...)
{
    va_list args;
    va_start(args, argc);
    for (int i = 0; i < argc; i++) {
        free(va_arg(args, void*));
    }
    va_end(args);
    if (status == EXIT_FAILURE) {
        printf("Image denoising failed!\n");
        printf("For more information, run the program with the --help option.\n");
        exit(status);
    }
    printf("Image successfully denoised!\n");
    exit(status);
}

int main(int argc, char* argv[])
{
    int v_opt = 0; // default choice of version is SIMD, can be changed with Option -V
    int b_opt = 1;
    int runtime = 0;
    char* input_path = NULL;
    char* output_path = "output.pgm"; // default output path, can be changed with Option -o
    float coeff[3] = { 0.2126, 0.7152, 0.0722 }; // default choice of coefficients for greyscale conversion, can be changed with Option --coeffs
    void (*denoise_sisd)(const uint8_t*, size_t, size_t, float, float, float, uint8_t*, uint8_t*, uint8_t*) = NULL;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "V:B::c:o:th", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'V':
            v_opt = parseX(optarg, "-V");
            if (v_opt == -1)
                return EXIT_FAILURE;
            break;
        case 'B':
            runtime = 1;
            if (optarg == NULL) {
                if (optind < argc && IS_DIGIT(argv[optind][0])) {
                    optarg = argv[optind++];
                } else {
                    break;
                }
            }
            b_opt = parseX(optarg, "-B");
            if (b_opt == -1)
                return EXIT_FAILURE;
            break;
        case 'o':
            if (optarg != NULL)
                output_path = optarg;
            break;
        case 'c':
            if (optarg == NULL || sscanf(optarg, "%f,%f,%f", &coeff[0], &coeff[1], &coeff[2]) != 3) {
                fprintf(stderr, "Could not parse argument for option --coeffs!\n");
                printf("For more information, run the program with the --help option.\n");
                return EXIT_FAILURE;
            }
            break;
        case 't':
            if (run_all_func_tests() + run_all_perf_tests())
                exit(EXIT_FAILURE);
            exit(EXIT_SUCCESS);
        case 'h':
        case '?': {
            FILE* file = fopen("help.txt", "r");
            char ch;
            while ((ch = fgetc(file)) != EOF) {
                printf("%c", ch);
            }
            printf("\n");
            fclose(file);
            if (opt == 'h')
                exit(EXIT_SUCCESS);
            else
                exit(EXIT_FAILURE);
        }
        }
    }
    if (optind < argc) {
        input_path = argv[optind];
    } else {
        fprintf(stderr, "No input file specified!\n");
        printf("For more information, run the program with the --help option.\n");
        return EXIT_FAILURE;
    }
    printf("Using coefficients %f, %f, %f while converting to grayscale\n", coeff[0], coeff[1], coeff[2]);

    // avoid dynamic memory on the heap to use exit() directly in read_image() if an error occurs
    struct Netpbm image;
    read_image(input_path, &image);

    // allocation of temporary image arrays
    uint8_t* tmp1 = malloc(image.width * image.height * sizeof(uint8_t));
    uint8_t* tmp2 = malloc(image.width * image.height * sizeof(uint8_t));
    uint8_t* result_pixels = malloc(image.width * image.height * sizeof(uint8_t));
    if (!tmp1 || !tmp2 || !result_pixels)
        cleanup_end(EXIT_FAILURE, 3, tmp1, tmp2, result_pixels);
    uint16_t* padded_image = NULL;
    uint16_t* padded_laplace = NULL;
    uint16_t* padded_blur = NULL;

    if (v_opt == 1 || v_opt == 2) {
        if (v_opt == 1) {
            denoise_sisd = denoise_integer;
            printf("Denoising the image %s using integer version of SISD...\n", input_path);
        } else {
            denoise_sisd = denoise;
            printf("Denoising the image %s using accurate version of SISD...\n", input_path);
        }
        if (runtime) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);
            for (int i = 0; i < b_opt; i++)
                denoise_sisd(image.pixels, image.width, image.height, coeff[0], coeff[1], coeff[2], tmp1, tmp2, result_pixels);
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
            printf("Time taken in total: %f second for %d iterations\n", time_taken, b_opt);
            printf("Time taken per iteration: %f second\n", time_taken / b_opt);
        } else {
            denoise_sisd(image.pixels, image.width, image.height, coeff[0], coeff[1], coeff[2], tmp1, tmp2, result_pixels);
        }
    } else {
        printf("Denoising the image %s using SIMD...\n", input_path);
        size_t padded_size = (image.width + 2) * (image.height + 2);
        padded_image = calloc(padded_size, sizeof(uint16_t));
        padded_laplace = calloc(padded_size, sizeof(uint16_t));
        padded_blur = calloc(padded_size, sizeof(uint16_t));

        if (!padded_image || !padded_laplace || !padded_blur)
            cleanup_end(EXIT_FAILURE, 6, tmp1, tmp2, result_pixels, padded_image, padded_laplace, padded_blur);

        if (runtime) {
            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);
            for (int i = 0; i < b_opt; i++)
                denoise_simd(image.pixels, image.width, image.height, coeff[0], coeff[1], coeff[2], padded_image, padded_laplace, padded_blur, result_pixels);
            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
            printf("Time taken in total: %f second for %d iterations\n", time_taken, b_opt);
            printf("Time taken per iteration: %f second\n", time_taken / b_opt);
        } else {
            denoise_simd(image.pixels, image.width, image.height, coeff[0], coeff[1], coeff[2], padded_image, padded_laplace, padded_blur, result_pixels);
        }
    }

    free(image.pixels);
    // store results in image struct
    image.pixels = result_pixels;
    image.magicNumber[1] = '5';

    if (write_image(&image, output_path) == EXIT_FAILURE)
        cleanup_end(EXIT_FAILURE, 6, tmp1, tmp2, result_pixels, padded_image, padded_laplace, padded_blur);

    cleanup_end(EXIT_SUCCESS, 6, tmp1, tmp2, result_pixels, padded_image, padded_laplace, padded_blur);
}