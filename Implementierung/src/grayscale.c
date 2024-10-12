#include "grayscale.h"
#include <math.h>
#include <smmintrin.h>

void grayscale(const uint8_t* image, size_t width, size_t height,
    float a, float b, float c, uint8_t* result)
{
    float a_f = a / (a + b + c);
    float b_f = b / (a + b + c);
    float c_f = c / (a + b + c);
    for (size_t i = 0; i < (width * height); i++) {
        result[i] = (uint8_t)round((image[i * 3] * a_f + image[(i * 3) + 1] * b_f + image[(i * 3) + 2] * c_f));
    }
}

void grayscale_integer(const uint8_t* image, size_t width, size_t height,
    float a, float b, float c, uint8_t* result)
{
    uint32_t a_i = (uint32_t)(a * 1024 / (a + b + c));
    uint32_t b_i = (uint32_t)(b * 1024 / (a + b + c));
    uint32_t c_i = (uint32_t)(c * 1024 / (a + b + c));
    for (size_t i = 0; i < (width * height); i++) {
        result[i] = (uint8_t)((image[i * 3] * a_i + image[(i * 3) + 1] * b_i + image[(i * 3) + 2] * c_i) / 1024);
    }
}

void grayscale_simd(const uint8_t* image, size_t width, size_t height, float a, float b, float c, uint8_t* result)
{

    size_t size = width * height, i = 0;
    __m128 red_coeff = _mm_set1_ps(a), green_coeff = _mm_set1_ps(b), blue_coeff = _mm_set1_ps(c); // coeffs
    __m128 sumOfCoeffs = _mm_add_ps(_mm_add_ps(red_coeff, green_coeff), blue_coeff);
    red_coeff = _mm_div_ps(red_coeff, sumOfCoeffs);
    green_coeff = _mm_div_ps(green_coeff, sumOfCoeffs);
    blue_coeff = _mm_div_ps(blue_coeff, sumOfCoeffs);

    for (; i < size - 5; i += 4) {
        // load 128bits
        __m128i rgb = _mm_loadu_si128((const __m128i*)(&image[i * 3]));
        // rearranging channels
        __m128i mask = _mm_set_epi8(
            15, 14, 13, 12, // ignored
            11, 8, 5, 2, // blue
            10, 7, 4, 1, // green
            9, 6, 3, 0 // red
        );

        __m128i rgb_rearr = _mm_shuffle_epi8(rgb, mask);

        __m128 red_scaled = _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(rgb_rearr)), red_coeff);
        __m128 green_scaled = _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(rgb_rearr, 4))), green_coeff);
        __m128 blue_scaled = _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_srli_si128(rgb_rearr, 8))), blue_coeff);

        __m128i res = _mm_cvtps_epi32(_mm_add_ps(_mm_add_ps(red_scaled, green_scaled), blue_scaled));

        // converting 32bits int to uint8
        _mm_storel_epi64((__m128i*)(result + i), _mm_packus_epi16(_mm_packs_epi32(res, res), res));
    }
    // remaining pixels
    for (; i < size; i++) {
        result[i] = (uint8_t)((image[i * 3] * a + image[(i * 3) + 1] * b + image[(i * 3) + 2] * c) / (a + b + c));
    }
}