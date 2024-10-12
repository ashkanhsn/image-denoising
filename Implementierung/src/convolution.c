#include "convolution.h"
#include <math.h>
#include <smmintrin.h>

#define laplace_accurate(sum) ((uint8_t)round(abs(sum) / 4.0))
#define blur_accurate(sum) ((uint8_t)round(sum / 16.0))

int16_t blur_kernel[9] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 };
int16_t laplace_kernel[9] = { 0, 1, 0, 1, -4, 1, 0, 1, 0 };

// Naive implementation of convolution
void convolution(const uint8_t* image, size_t width, size_t height, uint8_t* result, const int16_t* kernel, int laplace)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = y * width + x;
            int16_t sum = 0;
            for (int16_t i = -1; i <= 1; i++) {
                for (int16_t j = -1; j <= 1; j++) {
                    if ((int64_t)x + i < 0 || (int64_t)y + j < 0 || x + i >= width || y + j >= height) {
                        continue;
                    }
                    sum += (int16_t)image[index + i + j * width] * kernel[(i + 1) + (j + 1) * 3];
                }
            }
            result[index] = laplace ? laplace_accurate(sum) : blur_accurate(sum);
        }
    }
}

// First Optimization: Perform two convolutions in one pass
void convolution_1pass(const uint8_t* image, size_t width, size_t height, uint8_t* result_laplace, uint8_t* result_blur)
{
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t index = y * width + x;
            int16_t sum_blur = 0, sum_laplace = 0;
            for (int16_t i = -1; i <= 1; i++) {
                for (int16_t j = -1; j <= 1; j++) {
                    if ((int64_t)x + i < 0 || (int64_t)y + j < 0 || x + i >= width || y + j >= height) {
                        continue;
                    }
                    sum_blur += (int16_t)image[index + i + j * width] * blur_kernel[(i + 1) + (j + 1) * 3];
                    sum_laplace += (int16_t)image[index + i + j * width] * laplace_kernel[(i + 1) + (j + 1) * 3];
                }
            }
            result_blur[index] = (uint8_t)(sum_blur / 16);
            result_laplace[index] = (uint8_t)(abs(sum_laplace) / 4);
        }
    }
}

// Second Optimization: Use SIMD to perform convolution
void convolution_simd(const uint16_t* padded_image, size_t padded_width, size_t padded_height, uint16_t* padded_laplace, uint16_t* padded_blur)
{
    size_t padded_size = padded_width * padded_height;
    size_t i = 0;
    __m128i negate_mask = _mm_set1_epi16(-1);
    // till loading data may cause an undefined behavior because of out of bound access
    for (; i < padded_size - padded_width * 2 - 9; i += 8) {
        // apply the kernel: {0, 1, 0, 1, -4, 1, 0, 1, 0}
        __m128i x0y1 = _mm_loadu_si128((__m128i*)&padded_image[i + padded_width]);
        __m128i x1y0 = _mm_loadu_si128((__m128i*)&padded_image[i + 1]);
        __m128i x1y1 = _mm_slli_epi16(_mm_loadu_si128((__m128i*)&padded_image[i + padded_width + 1]), 2); // *4
        __m128i x1y2 = _mm_loadu_si128((__m128i*)&padded_image[i + 2 * padded_width + 1]);
        __m128i x2y1 = _mm_loadu_si128((__m128i*)&padded_image[i + padded_width + 2]);
        __m128i x1y1_laplace = _mm_sign_epi16(x1y1, negate_mask); // negate to get *-4
        __m128i sum_laplace = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(x0y1, x1y0), x1y1_laplace), x1y2), x2y1);
        sum_laplace = _mm_srli_epi16(_mm_abs_epi16(sum_laplace), 2);
        _mm_storeu_si128((__m128i*)&padded_laplace[i + padded_width + 1], sum_laplace);

        // apply the kernel: {1, 2, 1, 2, 4, 2, 1, 2, 1} ...
        __m128i x0y0 = _mm_loadu_si128((__m128i*)&padded_image[i]);
        __m128i x0y2 = _mm_loadu_si128((__m128i*)&padded_image[i + 2 * padded_width]);
        __m128i x2y0 = _mm_loadu_si128((__m128i*)&padded_image[i + 2]);
        __m128i x2y2 = _mm_loadu_si128((__m128i*)&padded_image[i + 2 * padded_width + 2]);
        x0y1 = _mm_slli_epi16(x0y1, 1);
        x1y0 = _mm_slli_epi16(x1y0, 1);
        x1y2 = _mm_slli_epi16(x1y2, 1);
        x2y1 = _mm_slli_epi16(x2y1, 1);

        __m128i sum_blur = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(x0y0, x0y1), _mm_add_epi16(x0y2, x1y0)), _mm_add_epi16(_mm_add_epi16(x1y1, x1y2), _mm_add_epi16(x2y0, x2y1))), x2y2);
        sum_blur = _mm_srli_epi16(sum_blur, 4);
        _mm_storeu_si128((__m128i*)&padded_blur[i + padded_width + 1], sum_blur);
    }
    // apply the kernel to the remaining pixels
    for (i = i + padded_width + 1; i < padded_size - padded_width - 1; i++) {
        int16_t sum_blur = 0, sum_laplace = 0;
        for (int j = 0; j < 9; j++) {
            sum_blur += (int16_t)padded_image[i + j % 3 - 1 + (j / 3 - 1) * padded_width] * blur_kernel[j];
            sum_laplace += (int16_t)padded_image[i + j % 3 - 1 + (j / 3 - 1) * padded_width] * laplace_kernel[j];
        }
        padded_blur[i] = (uint8_t)(sum_blur / 16);
        padded_laplace[i] = (uint8_t)(abs(sum_laplace) / 4);
    }
}

// ----- Helper functions for SIMD -----
// Have to write SIMD code since gcc auto-vectorization with O2 uses up to SSE2 but SSE4.1 is needed for _mm_cvtepu8_epi16
void pad_image_simd(const uint8_t* img, size_t width, size_t height, size_t padded_width, uint16_t* padded_image)
{
    size_t aligned = width - width % 16;
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < aligned; x += 16) {
            __m128i pix_8b = _mm_loadu_si128((__m128i*)&img[x + y * width]);
            __m128i pix_16b_low = _mm_cvtepu8_epi16(pix_8b);
            __m128i pix_16b_high = _mm_unpackhi_epi8(pix_8b, _mm_setzero_si128());
            _mm_storeu_si128((__m128i*)&padded_image[x + 1 + (y + 1) * padded_width], pix_16b_low);
            _mm_storeu_si128((__m128i*)&padded_image[x + 9 + (y + 1) * padded_width], pix_16b_high);
        }
        for (size_t x = aligned; x < width; x++) {
            padded_image[x + 1 + (y + 1) * padded_width] = img[x + y * width];
        }
    }
}

// ----- Blur in 2 passes -----
void blur_2_1d(const uint8_t* image, size_t width, size_t height, uint16_t* tmp, uint8_t* result)
{
    const uint16_t kernel[3] = { 1, 2, 1 };
    // blur in the x direction(horizontal)
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            uint16_t sum = 0;
            if ((int64_t)x - 1 >= 0)
                sum += image[x - 1 + y * width] * kernel[0];
            sum += image[x + y * width] * kernel[1];
            if (x + 1 < width)
                sum += image[x + 1 + y * width] * kernel[2];
            // store the sum in the transpoed position in tmp
            // for better cache locality when blurring in the y direction(vertical)
            tmp[x * height + y] = sum;
        }
    }
    // blur in the y direction(vertical)
    for (size_t y = 0; y < width; y++) {
        for (size_t x = 0; x < height; x++) {
            uint16_t sum = 0;
            if ((int64_t)x - 1 >= 0)
                sum += tmp[x - 1 + y * height] * kernel[0];
            sum += tmp[x + y * height] * kernel[1];
            if (x + 1 < height)
                sum += tmp[x + 1 + y * height] * kernel[2];
            result[x * width + y] = (uint8_t)(sum / 16);
        }
    }
}