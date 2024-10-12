#include "combine.h"
#include <emmintrin.h>
#include <math.h>

#define combine_acc(sum) (round(sum / 255.0))
#define combine_int(sum) (sum / 255)

// laplace convolution already converts it's results to an intermediary result that can be stored in a uint8_t array
void combine(const uint8_t* original, const uint8_t* laplace, const uint8_t* blur, size_t width, size_t height, uint8_t* result, int accurate)
{
    size_t size = width * height;
    for (size_t i = 0; i < size; i++) {
        int sum = laplace[i] * original[i] + (255 - laplace[i]) * blur[i];
        result[i] = (uint8_t)(accurate ? combine_acc(sum) : combine_int(sum));
    }
}

void combine_simd(const uint8_t* original, const uint16_t* padded_laplace, const uint16_t* padded_blur,
    size_t width, size_t height, size_t padded_width, uint8_t* result)
{
    size_t aligned = width - width % 16;
    __m128i i255 = _mm_set1_epi16(255);

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < aligned; x += 16) {
            __m128i original_8b = _mm_loadu_si128((__m128i*)&original[x + y * width]);

            __m128i original_16b_low = _mm_unpacklo_epi8(original_8b, _mm_setzero_si128());
            __m128i laplace_16b_low = _mm_loadu_si128((__m128i*)&padded_laplace[x + 1 + (y + 1) * padded_width]);
            __m128i blur_16b_low = _mm_loadu_si128((__m128i*)&padded_blur[x + 1 + (y + 1) * padded_width]);
            __m128i res_low = _mm_mullo_epi16(laplace_16b_low, original_16b_low);
            res_low = _mm_add_epi16(res_low, _mm_mullo_epi16(_mm_sub_epi16(i255, laplace_16b_low), blur_16b_low));
            res_low = _mm_srli_epi16(res_low, 8);

            __m128i original_16b_high = _mm_unpackhi_epi8(original_8b, _mm_setzero_si128());
            __m128i laplace_16b_high = _mm_loadu_si128((__m128i*)&padded_laplace[x + 9 + (y + 1) * padded_width]);
            __m128i blur_16b_high = _mm_loadu_si128((__m128i*)&padded_blur[x + 9 + (y + 1) * padded_width]);
            __m128i res_high = _mm_mullo_epi16(laplace_16b_high, original_16b_high);
            res_high = _mm_add_epi16(res_high, _mm_mullo_epi16(_mm_sub_epi16(i255, laplace_16b_high), blur_16b_high));
            res_high = _mm_srli_epi16(res_high, 8);

            _mm_storeu_si128((__m128i*)&result[x + y * width], _mm_packus_epi16(res_low, res_high));
        }
        for (size_t x = aligned; x < width; x++) {
            int sum = padded_laplace[x + 1 + (y + 1) * padded_width] * original[x + y * width] + (255 - padded_laplace[x + 1 + (y + 1) * padded_width]) * padded_blur[x + 1 + (y + 1) * padded_width];
            result[x + y * width] = (uint8_t)combine_int(sum);
        }
    }
}