// bmp.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // for abs()
#include "bmp.h"

// Encoder
Image* read_bmp(const char* filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }

    printf("[Info] Reading BMP file: %s\n", filename);
    
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    // 1. read headers
    fread(&fileHeader, sizeof(BMPFileHeader), 1, f);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, f);

    // 2. validate (only support 24-bit BMP)
    if (fileHeader.bfType != 0x4D42) { // Check the 'BM' signature
        printf("Error: Not a BMP file.\n");
        fclose(f);
        return NULL;
    }
    if (infoHeader.biBitCount != 24) { // Only 24-bit BMP
        printf("Error: Only 24-bit BMP is supported.\n");
        fclose(f);
        return NULL;
    }
    if (infoHeader.biPlanes != 1) { 
        printf("Error: Planes must be 1.\n");
        fclose(f);
        return NULL;
    }

    // 3. allocate Image 
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = infoHeader.biWidth;
    img->height = abs(infoHeader.biHeight); // handle negative Height
    
    int num_pixels = img->width * img->height; // total pixels = width * height
    img->data_r = (uint8_t*)malloc(num_pixels);
    img->data_g = (uint8_t*)malloc(num_pixels);
    img->data_b = (uint8_t*)malloc(num_pixels);

    // 4. calculate Padding
    // each row's byte count must be a multiple of 4 bytes
    int row_bytes = img->width * 3; // row bytes = 3 bytes per pixel (BGR) * width
    int padding = (4 - (row_bytes % 4)) % 4; // padding bytes 0 - 3 (%4 for case already multiple of 4, don't add 4 padding)

    // 5. seek to pixel data start
    fseek(f, fileHeader.bfOffBits, SEEK_SET);

    // 6. read loop (BGR, left to right, bottom to top if biHeight > 0)
    uint8_t pixel[3]; // Buffer for B, G, R
    
    for (int y = 0; y < img->height; y++) {
        // If biHeight > 0 (standard BMP), the first row in the file is the bottom row
        // We need to store it in the last row of memory, or do a vertical flip
        // let (0,0) be top-left in memory
        int target_y = (img->height - 1) - y; // flip vertically

        for (int x = 0; x < img->width; x++) {
            if (fread(pixel, 1, 3, f) != 3) break; // read BGR

            // Separate channels and convert from BGR to RGB
            // data[row * width + col]
            int idx = target_y * img->width + x;
            
            img->data_b[idx] = pixel[0]; // Blue
            img->data_g[idx] = pixel[1]; // Green
            img->data_r[idx] = pixel[2]; // Red
        }

        // skip Padding
        if (padding > 0) {
            fseek(f, padding, SEEK_CUR); // skip padding bytes from current position
        }
    }

    fclose(f);
    printf("[Info] Read BMP finished : %dx%d, Padding: %d\n", img->width, img->height, padding);
    return img;
}

void free_image(Image* img) {
    if (img) {
        if (img->data_r) free(img->data_r);
        if (img->data_g) free(img->data_g);
        if (img->data_b) free(img->data_b);
        free(img);
    }
}

void write_channel_txt(const char* filename, uint8_t* data, int width, int height) {
    FILE *f = fopen(filename, "w");
    if (!f) return; // check file open
    
    printf("[Info] Writing channel to %s\n", filename);

    for (int y = 0; y < height; y++) { // same as real image order
        for (int x = 0; x < width; x++) {
            fprintf(f, "%d ", data[y * width + x]); // write pixel value at image position
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void write_dim_txt(const char* filename, int width, int height) {
    FILE *f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%d %d", width, height);
        fclose(f);
    }
}

// Decoder
// read dimensions from text file
int read_dim_txt(const char* filename, int *width, int *height) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    printf("[Info] Reading dimensions from %s\n", filename);
    // read width and height to pointers
    if (fscanf(f, "%d %d", width, height) != 2) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

// read single channel data 
int read_channel_txt(const char* filename, uint8_t* data, int width, int height) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;
    printf("[Info] Reading channel data from %s\n", filename);
    
    // read pixel values
    for (int i = 0; i < width * height; i++) {
        int val;
        // read integer value
        if (fscanf(f, "%d", &val) != 1) {
            fclose(f);
            return 0;
        }
        // store to data array
        data[i] = (uint8_t)val;
    }
    fclose(f);
    return 1;
}

void write_bmp(const char* filename, Image* img) {
    FILE *f = fopen(filename, "wb"); // write binary
    if (!f) {
        printf("Error: Cannot open %s for writing.\n", filename);
        return;
    }
    printf("[Info] Writing BMP file: %s\n", filename);
    // 1. Calculate Padding and Size
    int row_bytes = img->width * 3;
    int padding = (4 - (row_bytes % 4)) % 4; // Padding bytes per row (same as read)
    
    // Total Image Data size
    uint32_t data_size = (row_bytes + padding) * img->height; // width*3 + padding per row, times height
    // Total file size = Header (54 bytes) + Data
    uint32_t file_size = 54 + data_size;

    // 2. Fill File Header
    BMPFileHeader fileHeader;
    fileHeader.bfType = 0x4D42;  // 'BM'
    fileHeader.bfSize = file_size; // Total file size
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = 54;   // Standard Offset

    // 3. Fill Info Header
    BMPInfoHeader infoHeader;
    infoHeader.biSize = 40;
    infoHeader.biWidth = img->width;
    infoHeader.biHeight = img->height; // Positive for Bottom-Up
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0; // BI_RGB
    infoHeader.biSizeImage = 0; // read in is 0 
    infoHeader.biXPelsPerMeter = 3780;  // Same DPI
    infoHeader.biYPelsPerMeter = 3780;  
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    // 4. Write Header
    fwrite(&fileHeader, sizeof(BMPFileHeader), 1, f); // write file header to f
    fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, f);

    // 5. Write pixel data 
    uint8_t pad_byte = 0;
    printf("[Info] Writing pixel data\n");
    // Bottom-Up BMP: Write from the last row in memory
    // img->height - 1 in memory
    for (int y = 0; y < img->height; y++) { // reverse order
        int src_y = (img->height - 1) - y; // start from the bottom in memory

        for (int x = 0; x < img->width; x++) { // from left to right
            int idx = src_y * img->width + x; // pixel index
            
            // Write order: B, G, R
            fwrite(&img->data_b[idx], 1, 1, f);
            fwrite(&img->data_g[idx], 1, 1, f);
            fwrite(&img->data_r[idx], 1, 1, f);
        }

        // Write Padding (fill 0)
        for (int k = 0; k < padding; k++) {
            fwrite(&pad_byte, 1, 1, f);
        }
    }

    fclose(f);
    printf("[Info] Wrote BMP finished: %s (%dx%d)\n", filename, img->width, img->height);
}