#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <stdint.h>
#include <stdlib.h>

extern int16_t blur_kernel[9];
extern int16_t laplace_kernel[9];

/**
 * Perform a convolution on an image with a 3x3 kernel.
 * Use the function fn to convert the sum to a pixel value.
 */
void convolution(const uint8_t* image, size_t width, size_t height, uint8_t* result, const int16_t* kernel, int laplace);
/**
 * Perform two convolutions on an image, one with a 2D laplace kernel and one with a 2D gaussian kernel.
 * Laplace filter is used for edge detection and gaussian filter is used to blur the image to reduce noise.
 * Two convolutions are performed in one pass to avoid iterating over the image twice.
 */
void convolution_1pass(const uint8_t* image, size_t width, size_t height, uint8_t* result_laplace, uint8_t* result_blur);

/**
 * Does the same as convolution(), but optimized using SSE, SSE4.1 is required.
 * Integer arithmetic is used for better performance, but the result is less accurate.
 * Allocate (width + 2) * (height + 2) pixels for arrays related to padded image.
 * @param padded_image: pointer to the padded image. The image must be padded before calling this function, can be done using pad_image_simd().
 */
void convolution_simd(const uint16_t* padded_image, size_t padded_width, size_t padded_height, uint16_t* padded_laplace, uint16_t* padded_blur);

/**
 * Make use of the separability of the gaussian kernel to perform the blur in two 1D passes.
 * Integer arithmetic is used for better performance, but the result is less accurate.
 */
void blur_2_1d(const uint8_t* image, size_t width, size_t height, uint16_t* tmp, uint8_t* result);

/**
 * Helper function to pad an image with zeros to avoid accessing wrong pixel values
 * which also simply the convolution in SIMD with a 3x3 kernel
 * and also convert the pixels to uint16_t
 */
void pad_image_simd(const uint8_t* img, size_t width, size_t height, size_t padded_width, uint16_t* padded_image);

#endif // CONVOLUTION_H