#ifndef GRAYSCALE_H
#define GRAYSCALE_H
#include <stddef.h>
#include <stdint.h>
/**
 * Converts an 24bpp RGB image to a 8bpp grayscale image
 * Results are calculated with floating point arithmetic and rounded accurately.
 * @param image: pointer to the RGB image (3 bytes per pixel)
 * @param width: width of the image
 * @param height: height of the image
 * @param a: weight of the red channel, de
 * @param b: weight of the green channel
 * @param c: weight of the blue channel
 * @param result: pointer to the result grayscale image
 */
void grayscale(const uint8_t* image, size_t width, size_t height, float a, float b, float c, uint8_t* result);

/**
 * Does the same as grayscale()
 * Because of integer arithmetic it is faster but less accurate
 * @param image: pointer to the RGB image (3 bytes per pixel)
 * @param width: width of the image
 * @param height: height of the image
 * @param a: weight of the red channel, de
 * @param b: weight of the green channel
 * @param c: weight of the blue channel
 * @param result: pointer to the result grayscale image
 */
void grayscale_integer(const uint8_t* image, size_t width, size_t height, float a, float b, float c, uint8_t* result);

/**
 * Converts a RGB image to a grayscale image using simd instruction.
 * @param image: pointer to the RGB image (3 bytes per pixel)
 * @param width: width of the image
 * @param height: height of the image
 * @param a: weight of the red channel, de
 * @param b: weight of the green channel
 * @param c: weight of the blue channel
 * @param result: pointer to the result grayscale image
 */
void grayscale_simd(const uint8_t* image, size_t width, size_t height, float a, float b, float c, uint8_t* result);

#endif