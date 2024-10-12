#ifndef DENOISE_H
#define DENOISE_H
#include <stdint.h>
#include <stdlib.h>

/**
 * Function to convert an RGB image to a grayscale image and reduce the noise of the grayscale image.
 * This implementation is naive and the pixel value is rounded correctly throughout the process to get an accurate result.
 * @param img: pointer to the original RGB image
 * @param width: width of the image
 * @param height: height of the image
 * @param a: weight of the red channel
 * @param b: weight of the green channel
 * @param c: weight of the blue channel
 * @param tmp1: pointer to a temporary result
 * @param tmp2: pointer to a temporary result
 * @param result: pointer to the denoised grayscale image
 */
void denoise(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint8_t* tmp1, uint8_t* tmp2,
    uint8_t* result);

/**
 * Does the same as denoise().
 * Optimized for speed by using integer arithmetic and doing two convolutions in one pass.
 * The pixel value of the result is not exact, it may vary ±1, but the difference is not noticeable to human eyes.
 */
void denoise_integer(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint8_t* tmp1, uint8_t* tmp2,
    uint8_t* result);

/**
 * Does the samen as denoise(), optimized using SSE, SSE4.1 is required.
 * The result is not exact, it may vary ±2, but the difference is not noticeable to human eyes.
 * Allocate (width + 2) * (height + 2) pixels for arrays related to padded image.
 * @param img: pointer to the original RGB image
 * @param width: width of the image
 * @param height: height of the image
 * @param a: weight of the red channel
 * @param b: weight of the green channel
 * @param c: weight of the blue channel
 * @param tmp1: pointer to a temporary result
 * @param tmp2: pointer to a temporary result
 * @param padded_image: pointer to the padded image
 * @param padded_laplace: pointer to a temporary padded result
 * @param padded_blur: pointer to a temporary padded result
 * @param result: pointer to the denoised grayscale image
 */
void denoise_simd(const uint8_t* img, size_t width, size_t height,
    float a, float b, float c,
    uint16_t* padded_image, uint16_t* padded_laplace, uint16_t* padded_blur,
    uint8_t* result);

#endif