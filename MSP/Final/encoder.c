// encoder.c 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "common.h" 
#include "bmp.h"
#include "transform.h"
#include "huffman.h"

// Standard Quantization Tables 

// Luminance Quantization Table (for Y channel)
static int std_lum_qt[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

// Chrominance Quantization Table (for Cb, Cr channel)
static int std_chr_qt[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};

// Zigzag order for 8x8 block
static const int zigzag_order[64] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// Scale quantization table based on quality factor (1-100)
void scale_table(int base[8][8], int out[8][8], int q) {
    int scale;
    if (q <= 0) q = 1; // bound check
    if (q > 100) q = 100;
    // scaling formula (quality default 50 -> scale 100)
    if (q < 50) scale = 5000 / q; // lower quality
    else scale = 200 - 2 * q; // higher quality
    // update quantization table
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            long val = (long)base[i][j] * scale / 100; // scaled value
            if (val < 1) val = 1;
            if (val > 255) val = 255;
            out[i][j] = (int)val; // store in output table
        }
    }
}

// write quantization table to text file (for checking)
void write_qt_txt(const char* filename, int qt[8][8]) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Error: Cannot open %s for writing\n", filename);
        return;
    }
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            fprintf(f, "%d ", qt[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

// Mode 2:
// Encode single 8x8 quantized block with RLE and DPCM for DC
// q_block: input quantized 8x8 block
// prev_dc: pointer to previous DC value for DPCM
// m, n: block position ; channel name for ASCII mode
// long: return number of bytes to caculate compression ratio
long encode_block_rle(int16_t q_block[8][8], int16_t *prev_dc, FILE *f_out, int is_binary, int m, int n, const char* ch_name) {
    long bytes_written = 0;

    // 1. DPCM for DC coefficient (index 0)
    int16_t dc_val = q_block[0][0]; // current DC value (the first element)
    // int16_t dc_diff = dc_val - *prev_dc; // DC difference
    *prev_dc = dc_val; // update previous DC value

    // 2. Write DC difference
    if (is_binary) {
        // Binary : 
        uint8_t run = 0; // DC has no run
        fwrite(&run, sizeof(uint8_t), 1, f_out); // add Run
        fwrite(&dc_val, sizeof(int16_t), 1, f_out); 
        bytes_written += 3; // 1 byte (Run) + 2 bytes (Level)
    } else {
        // ASCII ：($m,$n, Name) $diff ...
        // Block position and channel
        fprintf(f_out, "(%d,%d, %s) ", m, n, ch_name);
        // DC (skip 0)
        fprintf(f_out, "0 %d ", dc_val);
    }

    // 2. Zigzag scan and RLE for AC coefficients
    int zero_count = 0; // count of continuous zeros
    
    for (int k = 1; k < 64; k++) {
        // change zigzag order to 2D index
        int idx = zigzag_order[k]; // precomputed zigzag index
        int u = idx / 8; // row
        int v = idx % 8;
        
        int16_t ac_val = q_block[u][v];

        if (ac_val == 0) {
            zero_count++;
        } else {
            // non zero : output (Run, Level)
            if (is_binary) {
                // Binary: [Run(1byte)][Level(2bytes)]
                uint8_t run = (uint8_t)zero_count;
                fwrite(&run, sizeof(uint8_t), 1, f_out); // write run
                fwrite(&ac_val, sizeof(int16_t), 1, f_out); // write level
                bytes_written += 3; // 1 + 2 bytes
            } else {
                // ASCII: $skip $value
                fprintf(f_out, "%d %d ", zero_count, ac_val);
            }
            zero_count = 0; // reset zero count
        }
    }

    // 3. EOB (End of Block)
    // (0,0) to indicate end of block
    if (is_binary) {
        uint8_t run = 0; 
        int16_t val = 0;
        fwrite(&run, sizeof(uint8_t), 1, f_out);
        fwrite(&val, sizeof(int16_t), 1, f_out);
        bytes_written += 3;
    } else {
        fprintf(f_out, "0 0\n"); // EOB
    }

    return bytes_written;
}

// Mode 3: collect a block's statistics for Huffman coding
// q_block: quantized 8x8 block
// prev_dc: previous DC value (for DPCM)
// dc_table: DC stats table
// ac_table: AC stats table
void collect_huffman_stats(int16_t q_block[8][8], int16_t *prev_dc, HuffmanTable *dc_table, HuffmanTable *ac_table) {
    // 1. DC Component (DPCM)
    int16_t dc_val = q_block[0][0];
    int16_t dc_diff = dc_val - *prev_dc;
    *prev_dc = dc_val; // update previous DC
    // DC : run = 0, val = dc_diff
    add_symbol(dc_table, 0, dc_diff); 

    // 2. AC Components (RLE)
    int zero_count = 0;
    for (int k = 1; k < 64; k++) { // iterate AC coefficients 8x8
        int idx = zigzag_order[k]; // zigzag index to 2D coordinates
        int16_t val = q_block[idx / 8][idx % 8]; 
        
        if (val == 0) { // count zeros
            zero_count++;
        } else {
            // non-zero value, record (Run, Val)
            add_symbol(ac_table, zero_count, val);
            zero_count = 0; // reset count
        }
    }
    // 3. EOB (End of Block) -> (0, 0)
    add_symbol(ac_table, 0, 0); 
}

// Mode 3:
// Encode single 8x8 quantized block using Huffman coding
void encode_huffman_block(int16_t q_block[8][8], int16_t *prev_dc, 
                          HuffmanTable *dc_table, HuffmanTable *ac_table, 
                          FILE *f_out, int is_binary, uint8_t *bit_buf, int *bit_cnt) {
    // 1. DC Component
    int16_t dc_val = q_block[0][0];
    int16_t dc_diff = dc_val - *prev_dc;
    *prev_dc = dc_val;
    
    // Find Huffman code for DC difference
    HuffNode *node = find_symbol(dc_table, 0, dc_diff);
    if (node) {
        if (is_binary) {
            write_huffman_bits(f_out, node->code, bit_buf, bit_cnt);
        } else {
            fprintf(f_out, "%s", node->code); // ASCII mode
        }
    }

    // 2. AC Components
    int zero_count = 0;
    for (int k = 1; k < 64; k++) { // iterate AC coefficients 8x8
        int idx = zigzag_order[k]; // zigzag index to 2D coordinates
        int16_t val = q_block[idx / 8][idx % 8]; // get AC value
        
        if (val == 0) { // count zeros
            zero_count++;
        } else { // non-zero
            // find Huffman code for (Run, Val)
            node = find_symbol(ac_table, zero_count, val);
            if (node) {
                if (is_binary) write_huffman_bits(f_out, node->code, bit_buf, bit_cnt);
                else fprintf(f_out, "%s", node->code); // ASCII mode
            }
            zero_count = 0;
        }
    }
    // 3. EOB
    node = find_symbol(ac_table, 0, 0);
    if (node) {
        if (is_binary) write_huffman_bits(f_out, node->code, bit_buf, bit_cnt);
        else fprintf(f_out, "%s", node->code);
    }
}

void save_codebook_file(const char *prefix, const char *suffix, HuffmanTable *table, const char *internal_name) {
    char filename[256];
    // filename： prefix + suffix ("code" + "_Y_DC.txt" = "code_Y_DC.txt")
    snprintf(filename, sizeof(filename), "%s%s", prefix, suffix);

    FILE *f = fopen(filename, "w");
    if (f) {
        write_codebook(f, table, internal_name);
        fclose(f);
        printf("[Info] Codebook saved: %s\n", filename);
    } else {
        printf("[Error] Cannot write to %s\n", filename);
    }
}

// ---------------------------------------------
// Mode functions
int run_mode_0(int argc, char *argv[]) {
    // Mode 0: BMP to Text & check
    // encoder 0 Input.bmp R.txt G.txt B.txt dim.txt
    if (argc < 7) {
        printf("Usage: encoder 0 <input.bmp> <R.txt> <G.txt> <B.txt> <dim.txt>\n");
        return 1;
    }

    const char *input_bmp = argv[2];
    const char *out_r = argv[3];
    const char *out_g = argv[4];
    const char *out_b = argv[5];
    const char *out_dim = argv[6];

    // 1. read BMP file
    Image *img = read_bmp(input_bmp);
    if (!img) return 1;

    // 2. write to text files
    write_channel_txt(out_r, img->data_r, img->width, img->height);
    write_channel_txt(out_g, img->data_g, img->width, img->height);
    write_channel_txt(out_b, img->data_b, img->width, img->height);
    write_dim_txt(out_dim, img->width, img->height);

    // 3. free memory
    free_image(img); // free rgb array memory
    
    printf("[Success] Mode 0 files generated.\n");
    return 0;
}

int run_mode_1(int argc, char *argv[]) {
    // Mode 1: RGB->YCbCr->DCT->Quantization
    // Process each channel: RGB -> YCbCr -> DCT -> Quantization -> Error Calculation
    // check arguments
    if (argc < 13) {
        printf("Usage: encoder 1 <in.bmp> <Qt_Y.txt> <Qt_Cb.txt> <Qt_Cr.txt> <dim.txt> <qF_Y.raw> <qF_Cb.raw> <qF_Cr.raw> <eF_Y.raw> <eF_Cb.raw> <eF_Cr.raw>\n");
        return 1;
    }

    // get arguments
    const char *input_bmp = argv[2];
    const char *out_qt_y = argv[3];
    const char *out_qt_cb = argv[4];
    const char *out_qt_cr = argv[5];
    const char *out_dim = argv[6];
    const char *out_qf_y = argv[7];
    const char *out_qf_cb = argv[8];
    const char *out_qf_cr = argv[9];
    const char *out_ef_y = argv[10];
    const char *out_ef_cb = argv[11];
    const char *out_ef_cr = argv[12];

    // 1. read BMP file
    Image *img = read_bmp(input_bmp);
    if (!img) return 1;

    // 2. output quantization tables and dimensions
    write_qt_txt(out_qt_y, std_lum_qt); 
    write_qt_txt(out_qt_cb, std_chr_qt);
    write_qt_txt(out_qt_cr, std_chr_qt); // Cb and Cr use same table
    write_dim_txt(out_dim, img->width, img->height);

    // 3. Open binary files for writing quantized data and error data
    FILE *f_q_y = fopen(out_qf_y, "wb"); FILE *f_e_y = fopen(out_ef_y, "wb");
    FILE *f_q_cb = fopen(out_qf_cb, "wb"); FILE *f_e_cb = fopen(out_ef_cb, "wb");
    FILE *f_q_cr = fopen(out_qf_cr, "wb"); FILE *f_e_cr = fopen(out_ef_cr, "wb");

    // SQNR statistics variables (for all channels and all DCT coefficients)
    double signal_power[3][8][8] = {{{0}}}; // initialize to 0
    double noise_power[3][8][8] = {{{0}}};

    // 4. compute number of 8x8 blocks
    int blocks_w = (img->width + 7) / 8; // ceiling 
    int blocks_h = (img->height + 7) / 8;
    printf("[Info] Mode 1 Processing: %dx%d blocks\n", blocks_w, blocks_h);

    // 5. Process each 8x8 block
    float blk_y[8][8], blk_cb[8][8], blk_cr[8][8]; // YCbCr blocks
    float dct_y[8][8], dct_cb[8][8], dct_cr[8][8]; // DCT coefficients
    int16_t q_y[8][8], q_cb[8][8], q_cr[8][8];     // quantized coefficients
    float err_y[8][8], err_cb[8][8], err_cr[8][8]; // error blocks
    // iterate all blocks
    for (int by = 0; by < blocks_h; by++) { // block y index
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }

        for (int bx = 0; bx < blocks_w; bx++) { // block x index
            
            // A. Fill 8x8 RGB block and convert to YCbCr
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    // check boundary
                    if (real_x >= img->width) real_x = img->width - 1;
                    if (real_y >= img->height) real_y = img->height - 1;
                    // actual image index
                    int idx = real_y * img->width + real_x;

                    // RGB to YCbCr
                    rgb_to_ycbcr(img->data_r[idx], img->data_g[idx], img->data_b[idx],
                                    &blk_y[y][x], &blk_cb[y][x], &blk_cr[y][x]);
                }
            }

            // B. Y Channel (DCT -> Quantize -> Error)
            dct_8x8(blk_y, dct_y); // block -> DCT
            quantize_8x8(dct_y, std_lum_qt, q_y); // DCT, Quantization -> Quantized
            calculate_error_8x8(dct_y, q_y, std_lum_qt, err_y); // Error Calculation

            fwrite(q_y, sizeof(int16_t), 64, f_q_y); // Write quantized data
            fwrite(err_y, sizeof(float), 64, f_e_y); // Write error data

            // Accumulate SQNR
            for(int u=0; u<8; u++) for(int v=0; v<8; v++) {
                signal_power[0][u][v] += dct_y[u][v] * dct_y[u][v];
                noise_power[0][u][v] += err_y[u][v] * err_y[u][v];
            }

            // C. Cb Channel (use std_chr_qt)
            dct_8x8(blk_cb, dct_cb);
            quantize_8x8(dct_cb, std_chr_qt, q_cb);
            calculate_error_8x8(dct_cb, q_cb, std_chr_qt, err_cb);
            
            fwrite(q_cb, sizeof(int16_t), 64, f_q_cb);
            fwrite(err_cb, sizeof(float), 64, f_e_cb);
            
            for(int u=0; u<8; u++) for(int v=0; v<8; v++) {
                signal_power[1][u][v] += dct_cb[u][v] * dct_cb[u][v];
                noise_power[1][u][v] += err_cb[u][v] * err_cb[u][v];
            }

            // D. Cr Channel (use std_chr_qt)
            dct_8x8(blk_cr, dct_cr);
            quantize_8x8(dct_cr, std_chr_qt, q_cr);
            calculate_error_8x8(dct_cr, q_cr, std_chr_qt, err_cr);
            
            fwrite(q_cr, sizeof(int16_t), 64, f_q_cr);
            fwrite(err_cr, sizeof(float), 64, f_e_cr);

            for(int u=0; u<8; u++) for(int v=0; v<8; v++) {
                signal_power[2][u][v] += dct_cr[u][v] * dct_cr[u][v];
                noise_power[2][u][v] += err_cr[u][v] * err_cr[u][v];
            }
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    // 6. close files and free memory
    fclose(f_q_y); fclose(f_e_y);
    fclose(f_q_cb); fclose(f_e_cb);
    fclose(f_q_cr); fclose(f_e_cr);
    free_image(img);

    // 7. Print SQNR results (same as before)
    printf("\n=== Quantized SQNR Results (dB) ===\n");
    const char* ch_name[] = {"Y", "Cb", "Cr"};
    for(int ch=0; ch<3; ch++) {
        printf("Channel %s:\n", ch_name[ch]);
        for(int u=0; u<8; u++) {
            for(int v=0; v<8; v++) {
                double s = signal_power[ch][u][v];
                double n = noise_power[ch][u][v];
                // n=0 -> infinite dB, s=0 -> 0 dB
                // normal case: 10*log10(s/n)
                double db = (n==0) ? 999.9 : ((s==0) ? 0.0 : 10 * log10(s/n));
                printf("%5.1f ", db);
            }   
            printf("\n");
        }
    }
    printf("[Success] Mode 1 encoding completed.\n");
    return 0;
}

int run_mode_2(int argc, char *argv[]) {
    // Mode 2: zig-zag DPCM + RLE encoding
    // encoder 2 Kimberly.bmp [ascii/binary] rle_code.[txt/bin]
    if (argc < 5) {
        printf("Usage: encoder 2 <in.bmp> <ascii/binary> <output_file>\n");
        return 1;
    }

    // get arguments
    const char *input_bmp = argv[2];
    const char *format_str = argv[3];
    const char *output_file = argv[4];
    int is_binary = 0;

    // determine format ascii or binary
    if (strcmp(format_str, "binary") == 0) is_binary = 1;
    else if (strcmp(format_str, "ascii") == 0) is_binary = 0;
    else { printf("Unknown format: %s\n", format_str); return 1; }

    printf("[Info] Mode 2: %s output to %s\n", is_binary ? "Binary" : "ASCII", output_file);
    printf("[Info] Reading input BMP...\n");
    // 1. read BMP file
    Image *img = read_bmp(input_bmp);
    if (!img) return 1;
    // 2. open output file
    FILE *f_out = fopen(output_file, is_binary ? "wb" : "w"); // binary or text
    if (!f_out) { printf("Error opening output file\n"); return 1; }

    // write dimensions
    if (is_binary) {
        fwrite(&img->width, sizeof(int), 1, f_out);
        fwrite(&img->height, sizeof(int), 1, f_out);
    } else {
        fprintf(f_out, "%d %d\n", img->width, img->height); // text format
        // fprintf(f_out, "# Format: (m,n,Channel) DC(Skip 0) AC(Run,Level) ... EOB(0,0)\n");
    }
    // each 8x8 block
    int blocks_w = (img->width + 7) / 8; // number of 8x8 blocks
    int blocks_h = (img->height + 7) / 8;
    
    // DPCM needs previous DC values
    int16_t prev_dc_y = 0, prev_dc_cb = 0, prev_dc_cr = 0;
    long total_bytes = 0;

    // temporary buffers
    float blk_y[8][8], blk_cb[8][8], blk_cr[8][8]; // YCbCr blocks
    float dct_y[8][8], dct_cb[8][8], dct_cr[8][8]; // DCT coefficients
    int16_t q_y[8][8], q_cb[8][8], q_cr[8][8];     // quantized coefficients

    printf("[Info] Mode 2 Processing: %dx%d blocks\n", blocks_w, blocks_h);
    // process all blocks
    for (int by = 0; by < blocks_h; by++) {
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }

        for (int bx = 0; bx < blocks_w; bx++) {
            
            // 1. RGB -> YCbCr
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    // check boundary
                    if (real_x >= img->width) real_x = img->width-1;
                    if (real_y >= img->height) real_y = img->height-1;

                    // actual image index
                    int idx = real_y * img->width + real_x;
                    rgb_to_ycbcr(img->data_r[idx], img->data_g[idx], img->data_b[idx],
                                 &blk_y[y][x], &blk_cb[y][x], &blk_cr[y][x]);
                }
            }

            // 2. Y Channel
            dct_8x8(blk_y, dct_y); // DCT
            quantize_8x8(dct_y, std_lum_qt, q_y); // Quantization
            total_bytes += encode_block_rle(q_y, &prev_dc_y, f_out, is_binary, by, bx, "Y"); // RLE + DPCM, count bytes 

            // 3. Cb Channel
            dct_8x8(blk_cb, dct_cb);
            quantize_8x8(dct_cb, std_chr_qt, q_cb);
            total_bytes += encode_block_rle(q_cb, &prev_dc_cb, f_out, is_binary, by, bx, "Cb");

            // 4. Cr Channel
            dct_8x8(blk_cr, dct_cr);
            quantize_8x8(dct_cr, std_chr_qt, q_cr);
            total_bytes += encode_block_rle(q_cr, &prev_dc_cr, f_out, is_binary, by, bx, "Cr");
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);
    fclose(f_out);

    // calculate compression ratio (only for binary)
    if (is_binary) {
        long orig_size = img->width * img->height * 3 + 54; // original BMP size (RGB + header)
        FILE *f_size = fopen(output_file, "rb"); 
        fseek(f_size, 0, SEEK_END);              // move to end
        long comp_size = ftell(f_size);          // compressed file size
        fclose(f_size);
        printf("\n=== RLE Compression Stats ===\n");
        printf("Original Size: %ld bytes\n", orig_size);
        printf("Compressed Size: %ld bytes\n", comp_size);
        printf("Compression Ratio: %.2f%%\n", (double)comp_size / orig_size * 100.0);
    } else {
        printf("[Success] Mode 2 (ASCII) finished. Output: %s\n", output_file);
    }

    free_image(img);
    return 0;
}

int run_mode_3(int argc, char *argv[]) {
    // Usage: encoder 3 <in.bmp> <ascii/binary> <codebook_prefix> <huffman_out>
    if (argc < 6) {
        printf("Usage: encoder 3 <in.bmp> <ascii/binary> <codebook_prefix> <huffman_out>\n");
        return 1;
    }
    
    // parse arguments
    const char *input_bmp = argv[2];
    const char *format_str = argv[3];
    const char *codebook_prefix = argv[4];
    const char *output_file = argv[5];
    int is_binary = (strcmp(format_str, "binary") == 0);
    printf("[Info] Mode 3 Huffman Encoding (%s)...\n", format_str);

    // optional quality parameter
    int quality = 50; // default quality
    if (argc >= 7) {
        quality = atoi(argv[6]); // only change if user provides 7th argument
        printf("[Info] Mode 3 Quality: %d\n", quality);
    }

    // scale quantization tables based on quality
    int current_lum_qt[8][8];
    int current_chr_qt[8][8];
    scale_table(std_lum_qt, current_lum_qt, quality);
    scale_table(std_chr_qt, current_chr_qt, quality);
    
    printf("[Info] Reading input BMP...\n");
    // 1. read BMP file
    Image *img = read_bmp(input_bmp);
    if (!img) return 1;

    // Prepare 4 Huffman Tables (for Y DC, Y AC, Cb/Cr DC, Cb/Cr AC)
    HuffmanTable table_y_dc, table_y_ac;
    HuffmanTable table_c_dc, table_c_ac; // Cb/Cr use same tables
    init_huffman_table(&table_y_dc); // initialize
    init_huffman_table(&table_y_ac);
    init_huffman_table(&table_c_dc);
    init_huffman_table(&table_c_ac);

    int blocks_w = (img->width + 7) / 8; // number of 8x8 blocks
    int blocks_h = (img->height + 7) / 8;

    // temporary buffers
    float blk_y[8][8], blk_cb[8][8], blk_cr[8][8];
    float dct[8][8];
    int16_t q_y[8][8], q_cb[8][8], q_cr[8][8];
    
    // Step 1: Collect frequencies
    printf("[Step 1] Collecting frequencies...\n");
    int16_t prev_dc_y=0, prev_dc_cb=0, prev_dc_cr=0; // reset DPCM

    //iterate all blocks
    for (int by = 0; by < blocks_h; by++) {
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }
        for (int bx = 0; bx < blocks_w; bx++) {
            // 1. RGB -> YCbCr 
            for (int y=0; y<8; y++) for (int x=0; x<8; x++) {
                int rx = bx*8+x; int ry = by*8+y; // real x,y
                if(rx>=img->width) rx=img->width-1; 
                if(ry>=img->height) ry=img->height-1;
                int idx = ry*img->width+rx; // actual index
                rgb_to_ycbcr(img->data_r[idx], img->data_g[idx], img->data_b[idx], 
                             &blk_y[y][x], &blk_cb[y][x], &blk_cr[y][x]);
            }
            
            // DCT -> Quantization -> Collect Stats
            // 2. Y Channel : y_dc, y_ac
            dct_8x8(blk_y, dct); quantize_8x8(dct, current_lum_qt, q_y);
            collect_huffman_stats(q_y, &prev_dc_y, &table_y_dc, &table_y_ac);
            
            // 3. Cb Channel : c_dc, c_ac
            dct_8x8(blk_cb, dct); quantize_8x8(dct, current_chr_qt, q_cb);
            collect_huffman_stats(q_cb, &prev_dc_cb, &table_c_dc, &table_c_ac);
            
            // 4. Cr Channel : c_dc, c_ac
            dct_8x8(blk_cr, dct); quantize_8x8(dct, current_chr_qt, q_cr);
            collect_huffman_stats(q_cr, &prev_dc_cr, &table_c_dc, &table_c_ac);
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    // Build Tree & Write Codebook
    printf("[Info] Building Huffman Trees...\n");
    build_huffman_tree(&table_y_dc); // build trees by using collected frequencies
    build_huffman_tree(&table_y_ac);
    build_huffman_tree(&table_c_dc);
    build_huffman_tree(&table_c_ac);

    // Write codebook to file
    save_codebook_file(codebook_prefix, "_Y_DC.txt", &table_y_dc, "Y_DC");
    save_codebook_file(codebook_prefix, "_Y_AC.txt", &table_y_ac, "Y_AC");
    save_codebook_file(codebook_prefix, "_C_DC.txt", &table_c_dc, "C_DC");
    save_codebook_file(codebook_prefix, "_C_AC.txt", &table_c_ac, "C_AC");

    // Step 2: Encode with Huffman Codes
    printf("[Step 2] Encoding to %s...\n", output_file);
    FILE *f_out = fopen(output_file, is_binary ? "wb" : "w");
    if (!f_out) return 1;

    // Write Header (width, height)
    if (is_binary) {
        fwrite(&img->width, sizeof(int), 1, f_out);
        fwrite(&img->height, sizeof(int), 1, f_out);
    } else {
        fprintf(f_out, "%d %d\n", img->width, img->height);
    }

    uint8_t bit_buf = 0; // Bit buffer
    int bit_count = 0;   // Bit count in buffer
    
    // reset previous DCs (DPCM)
    prev_dc_y=0; prev_dc_cb=0; prev_dc_cr=0; 

    for (int by = 0; by < blocks_h; by++) {
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }
        for (int bx = 0; bx < blocks_w; bx++) {
            // 1. RGB -> YCbCr
            for (int y=0; y<8; y++) for (int x=0; x<8; x++) { // all pixels in block
                int rx = bx*8+x; int ry = by*8+y; // real x,y
                if(rx>=img->width) rx=img->width-1; 
                if(ry>=img->height) ry=img->height-1;
                int idx = ry*img->width+rx; //actual index
                rgb_to_ycbcr(img->data_r[idx], img->data_g[idx], img->data_b[idx], 
                             &blk_y[y][x], &blk_cb[y][x], &blk_cr[y][x]);
            }
            
            // Encode Y
            dct_8x8(blk_y, dct); quantize_8x8(dct, current_lum_qt, q_y);
            encode_huffman_block(q_y, &prev_dc_y, &table_y_dc, &table_y_ac, f_out, is_binary, &bit_buf, &bit_count);
            
            // Encode Cb
            dct_8x8(blk_cb, dct); quantize_8x8(dct, current_chr_qt, q_cb);
            encode_huffman_block(q_cb, &prev_dc_cb, &table_c_dc, &table_c_ac, f_out, is_binary, &bit_buf, &bit_count);
            
            // Encode Cr
            dct_8x8(blk_cr, dct); quantize_8x8(dct, current_chr_qt, q_cr);
            encode_huffman_block(q_cr, &prev_dc_cr, &table_c_dc, &table_c_ac, f_out, is_binary, &bit_buf, &bit_count);
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    // Flush remaining bits in buffer
    if (is_binary) {
        flush_huffman_bits(f_out, &bit_buf, &bit_count);
    }
    fclose(f_out);
    printf("[Success] Mode 3 Huffman encoding completed.\n");
    // Compression Ratio Calculation
    if (is_binary) { // only for binary mode
        long orig_size = img->width * img->height * 3 + 54; // original BMP size
        long comp_size = 0;
        FILE *f_check = fopen(output_file, "rb");
        if (f_check) {
            fseek(f_check, 0, SEEK_END);
            comp_size = ftell(f_check); // get compressed file size
            fclose(f_check);
            
            printf("\n=== Huffman Compression Stats ===\n");
            printf("Original Size: %ld bytes\n", orig_size);
            printf("Compressed Size: %ld bytes\n", comp_size);
            printf("Compression Ratio: %.2f%%\n", (double)comp_size / orig_size * 100.0);
        }
    }

    // Cleanup
    free_image(img);
    free_huffman_table(&table_y_dc);
    free_huffman_table(&table_y_ac);
    free_huffman_table(&table_c_dc);
    free_huffman_table(&table_c_ac);
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: encoder <mode> ...\n");
        return 1;
    }

    int mode = atoi(argv[1]); // string to int

    if (mode == 0) {
        return run_mode_0(argc, argv);
    }
    else if (mode == 1) {
        return run_mode_1(argc, argv);
    }
    else if (mode == 2) {
        return run_mode_2(argc, argv);
    }
    else if (mode == 3) {
        return run_mode_3(argc, argv);
    }
    else {
        printf("Unknown mode: %d\n", mode);
        return 1;
    }

    return 0;
}