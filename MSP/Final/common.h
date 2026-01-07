// common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdlib.h>

// BMP File Header (14 bytes) and Info Header (40 bytes)
// force 1-byte alignment, to match BMP file structure (no padding)
#pragma pack(push, 1) 

// BMP File Header save in little-endian (3624 2e02 => 0x 02 2e 24 36)
typedef struct {
    uint16_t bfType;      // 'BM'
    uint32_t bfSize;      // File Size (header + data)
    uint16_t bfReserved1; // Reserved, must be 0
    uint16_t bfReserved2; // Reserved, must be 0
    uint32_t bfOffBits;   // Offset to pixel data (54 for header)
} BMPFileHeader;

typedef struct {
    uint32_t biSize;          // Info Header Size (40)
    int32_t  biWidth;         // Width
    int32_t  biHeight;        // Height
    uint16_t biPlanes;        // Must be 1
    uint16_t biBitCount;      // 24 bits per pixels
    uint32_t biCompression;   // 0 (no compression)
    uint32_t biSizeImage;     // Image Size (may be 0 for no compression)
    int32_t  biXPelsPerMeter; // Resolution X (pixels per meter)
    int32_t  biYPelsPerMeter; // Resolution Y (pixels per meter)
    uint32_t biClrUsed;       // Color Table Size (palette)
    uint32_t biClrImportant;  // Important Color Count (0 = all important)
} BMPInfoHeader;

#pragma pack(pop)

// Planar Format (image data separated by R, G, B channels)
// separate R, G, B data 
typedef struct {
    int width;
    int height;
    uint8_t *data_r; // Red channel data
    uint8_t *data_g; // Green channel data
    uint8_t *data_b; // Blue channel data
} Image;

#endif