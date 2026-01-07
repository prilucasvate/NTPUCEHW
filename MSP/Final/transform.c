// transform.c
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "transform.h"

// C(u) for DCT and IDCT
float C(int u) {
    if (u == 0) return 1.0 / sqrt(2.0);
    else return 1.0;
}

// RGB to YCbCr 
void rgb_to_ycbcr(uint8_t r, uint8_t g, uint8_t b, float *y, float *cb, float *cr) {
    double R = (double)r;
    double G = (double)g;
    double B = (double)b;
    
    // JPEG RGB to YCbCr conversion matrix
    double Y  =  0.299f * R + 0.587f * G + 0.114f * B;
    // Cb and Cr range shifted by +128 (to handle negative values)
    double Cb = -0.1687f * R - 0.3313f * G + 0.5f * B + 128.0f;
    double Cr =  0.5f * R - 0.4187f * G - 0.0813f * B + 128.0f;

    *y  = (float)Y;
    *cb = (float)Cb;
    *cr = (float)Cr;
}
// YCbCr to RGB
void ycbcr_to_rgb(float y, float cb, float cr, uint8_t *r, uint8_t *g, uint8_t *b) {
    // Reverse the Cb/Cr shift to original
    double Y = (double)y;
    double Cb = (double)cb - 128.0;
    double Cr = (double)cr - 128.0;

    // Inverse conversion matrix
    double r_val = Y + 1.402 * Cr;
    double g_val = Y - 0.34414 * Cb - 0.71414 * Cr;
    double b_val = Y + 1.772 * Cb;

    // Clamp to [0, 255] 
    if (r_val < 0) r_val = 0; 
    if (r_val > 255) r_val = 255;
    if (g_val < 0) g_val = 0; 
    if (g_val > 255) g_val = 255;
    if (b_val < 0) b_val = 0; 
    if (b_val > 255) b_val = 255;

    // set output (needs rounding or may truncate)
    *r = (uint8_t)round(r_val);
    *g = (uint8_t)round(g_val);
    *b = (uint8_t)round(b_val);
}

// Original DCT (time complexity O(N^4))
// may optimize later with fast DCT
void dct_8x8(float input[8][8], float output[8][8]) {
    // u and v are frequency matrix indices
    // 8*8 DCT
    for (int u = 0; u < 8; ++u) { // output row
        for (int v = 0; v < 8; ++v) { // output column
            float sum = 0.0;
            // x and y are actual image block indices
            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    // DCT formula  
                    sum += (double)input[x][y] * cos(((2.0 * x + 1.0) * u * M_PI) / 16.0) * cos(((2.0 * y + 1.0) * v * M_PI) / 16.0);
                }
            }
            // Final DCT answer with normalization factor
            output[u][v] = (float)(0.25 * C(u) * C(v) * sum);
        }
    }
}

// IDCT : Inverse DCT
void idct_8x8(float input[8][8], float output[8][8]) {
    // reverse of DCT
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            float sum = 0.0;
            for (int u = 0; u < 8; ++u) {
                for (int v = 0; v < 8; ++v) {
                    sum += C(u) * C(v) * (double)input[u][v] * cos(((2.0 * x + 1.0) * u * M_PI) / 16.0) * cos(((2.0 * y + 1.0) * v * M_PI) / 16.0);
                }
            }
            output[x][y] = (float)(0.25 * sum);
        }
    }
}

// Quantization ：DCT coefficients / quantization table, round to nearest integer
void quantize_8x8(float dct_coef[8][8], int quant_table[8][8], int16_t out_quantized[8][8]) {
    for(int u=0; u<8; u++) { // row
        for(int v=0; v<8; v++) { // column
            // round( value / table_value )
            float val = dct_coef[u][v] / (float)quant_table[u][v];
            // round and cast to int16_t
            out_quantized[u][v] = (int16_t)round(val); 
        }
    }
}
// example: Quantization
// DCT coffefficient = 32, Q = 10 -> quantized = round(32/10) = 32/10 = 3
// example: Dequantization
// quantized = 3, Q = 10 -> dequantized = 3 * 10 = 30 (not exactly the original 32)

// Dequantization ： quantized value * quantization table (not same as original DCT coefficients)
void dequantize_8x8(int16_t quantized[8][8], int quant_table[8][8], float out_dct_coef[8][8]) {
    for(int u=0; u<8; u++) { // row
        for(int v=0; v<8; v++) { // column
            // F' = q * Q (loss the fractional part) 
            out_dct_coef[u][v] = quantized[u][v] * (float)quant_table[u][v];
        }
    }
}

// error = original DCT - dequantized DCT
void calculate_error_8x8(float dct_orig[8][8], int16_t quantized[8][8], int quant_table[8][8], float error[8][8]) {
    for(int u=0; u<8; u++) {
        for(int v=0; v<8; v++) {
            // e = F - q * Q 
            error[u][v] = dct_orig[u][v] - (quantized[u][v] * (float)quant_table[u][v]);
        }
    }
}