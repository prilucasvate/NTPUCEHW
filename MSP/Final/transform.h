// transform.h
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "common.h"

// Define M_PI 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// RGB <-> YCbCr
// RGB (0-255) to YCbCr (Y: 0-255, Cb/Cr: 0-255)
void rgb_to_ycbcr(uint8_t r, uint8_t g, uint8_t b, float *y, float *cb, float *cr);
void ycbcr_to_rgb(float y, float cb, float cr, uint8_t *r, uint8_t *g, uint8_t *b);


// 8x8 DCT and IDCT
void dct_8x8(float input[8][8], float output[8][8]);
void idct_8x8(float input[8][8], float output[8][8]);

// Quantization 
// q_table is 8x8 quantization table
void quantize_8x8(float dct_coef[8][8], int quant_table[8][8], int16_t out_quantized[8][8]);
void dequantize_8x8(int16_t quantized[8][8], int quant_table[8][8], float out_dct_coef[8][8]);

// Calculate Error (Residual)
// E = F - Q * q_table
void calculate_error_8x8(float dct_orig[8][8], int16_t quantized[8][8], int quant_table[8][8], float error[8][8]);

#endif