#ifndef COMBINE_H
#define COMBINE_H
#include <stddef.h>
#include <stdint.h>

/**
 * Function to combine results of blur and edge detection(laplace) with the original grayscale image
 * Results are calculated with floating point arithmetic and rounded accurately.
 * @param accurate if false, integer arithmetic is used, which is faster but less accurate
 */
void combine(const uint8_t* original, const uint8_t* laplace, const uint8_t* blur, size_t width, size_t height, uint8_t* result, int accurate);

/**
 * Does the same as combine(), optimized using SIMD, SSE4.1 is required
 * Also unpadding is done here, so the padded arrays, resulting from the convolution_simd(), are required
 * Integer arithmetic is used for better performance, but the result is less accurate.
 */
void combine_simd(const uint8_t* original, const uint16_t* padded_laplace, const uint16_t* padded_blur,
    size_t width, size_t height, size_t padded_width, uint8_t* result);

#endif