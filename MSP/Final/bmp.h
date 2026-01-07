// bmp.h
#ifndef BMP_H
#define BMP_H

#include "common.h"

// read BMP file and return Image struct pointer
Image* read_bmp(const char* filename);

// free Image memory
void free_image(Image* img);

// Encoder : write single color channel to text file (Mode 0)
void write_channel_txt(const char* filename, uint8_t* data, int width, int height);

// Encoder : write dimensions to text file (Mode 0)
void write_dim_txt(const char* filename, int width, int height);

// Decoder : write Image struct back to BMP file
void write_bmp(const char* filename, Image* img);

// Decoder : read data back from text file
int read_dim_txt(const char* filename, int *width, int *height);
int read_channel_txt(const char* filename, uint8_t* data, int width, int height);

#endif