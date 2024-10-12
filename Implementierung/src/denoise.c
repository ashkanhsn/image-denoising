#include "denoise.h"
#include "combine.h"
#include "convolution.h"
#include "grayscale.h"

void denoise(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint8_t* tmp1, uint8_t* tmp2, uint8_t* result)
{
    grayscale(img, width, height, a, b, c, result);
    convolution(result, width, height, tmp1, laplace_kernel, 1);
    convolution(result, width, height, tmp2, blur_kernel, 0);
    combine(result, tmp1, tmp2, width, height, result, 1);
}

void denoise_integer(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint8_t* tmp1, uint8_t* tmp2, uint8_t* result)
{
    grayscale_integer(img, width, height, a, b, c, result);
    convolution_1pass(result, width, height, tmp1, tmp2);
    combine(result, tmp1, tmp2, width, height, result, 0);
}

void denoise_simd(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint16_t* padded_image, uint16_t* padded_laplace, uint16_t* padded_blur, uint8_t* result)
{
    grayscale_simd(img, width, height, a, b, c, result);

    size_t padded_width = width + 2;
    size_t padded_height = height + 2;

    pad_image_simd(result, width, height, padded_width, padded_image);
    convolution_simd(padded_image, padded_width, padded_height, padded_laplace, padded_blur);
    combine_simd(result, padded_laplace, padded_blur, width, height, padded_width, result);
}