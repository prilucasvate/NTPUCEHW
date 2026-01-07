// decoder.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "common.h" 
#include "bmp.h"
#include "transform.h"
#include "huffman.h"

// For Mode 2 decoding: standard quantization tables and zigzag order
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

// read quantization table from text file
int read_qt_txt(const char* filename, int qt[8][8]) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            if (fscanf(f, "%d", &qt[i][j]) != 1) {
                fclose(f); return 0;
            }
        }
    }
    fclose(f);
    return 1;
}

// calculate image SQNR in pixel domain (Mode 1(a))
void calculate_image_sqnr(Image *orig, Image *recon) {
    if (orig->width != recon->width || orig->height != recon->height) {
        printf("[Warning] Image dimensions do not match for SQNR.\n");
        return;
    }

    double signal_sq[3] = {0}, noise_sq[3] = {0}; // R, G, B
    int total_pixels = orig->width * orig->height;

    for (int i = 0; i < total_pixels; i++) { // iterate all pixels
        // R channel
        signal_sq[0] += orig->data_r[i] * orig->data_r[i]; // signal power
        double err_r = (double)orig->data_r[i] - (double)recon->data_r[i]; // error = original - reconstructed
        noise_sq[0] += err_r * err_r; // noise power

        // G channel
        signal_sq[1] += orig->data_g[i] * orig->data_g[i]; // signal power
        double err_g = (double)orig->data_g[i] - (double)recon->data_g[i]; // error = original - reconstructed
        noise_sq[1] += err_g * err_g;

        // B channel
        signal_sq[2] += orig->data_b[i] * orig->data_b[i]; // signal power
        double err_b = (double)orig->data_b[i] - (double)recon->data_b[i]; // error = original - reconstructed
        noise_sq[2] += err_b * err_b; // noise power
    }

    printf("\n=== Reconstructed Image SQNR (dB) ===\n");
    const char *names[] = {"R", "G", "B"};
    for (int c = 0; c < 3; c++) {
        double db = 999.9;
        if (noise_sq[c] > 0) { // avoid division by zero
            if (signal_sq[c] == 0) db = 0.0; // no signal
            // normal case : 10*log10(S/N)
            else db = 10.0 * log10(signal_sq[c] / noise_sq[c]);
        }
        printf("Channel %s: %5.2f dB\n", names[c], db);
    }
}

// 3. Mode 2 ：decode RLE Block to Quantized Block (8x8)
void decode_block_rle(FILE *f_in, int16_t q_block[8][8], int is_binary) {
    // 1. initialize to 0
    memset(q_block, 0, sizeof(int16_t) * 64);

    // 2. skip block header for ASCII
    if (!is_binary) {
        // format: (m,n,Channel) read until ')'
        int c;
        while ( (c = fgetc(f_in)) != EOF ) {
            if (c == ')') break; 
        }
    }

    // 3. RLE decoding loop
    // We use k to track the current position in zigzag order (0~63)
    int k = 0; 
    while (k < 64) { // 8*8=64 coefficients
        int run = 0;
        int val = 0;

        if (is_binary) {
            uint8_t r;
            int16_t v;
            if (fread(&r, 1, 1, f_in) != 1) break; // read run
            if (fread(&v, 2, 1, f_in) != 1) break; // read level
            run = r;
            val = v;
        } else {
            // ASCII format: $run $val
            if (fscanf(f_in, "%d %d", &run, &val) != 2) break; 
        }

        // check for EOB
        // if run=0, val=0 且 k > 0， is EOB
        // (first coefficient cannot be EOB since DC must exist) 
        if (k > 0 && run == 0 && val == 0) {
            break; // EOB, finish this block
        }

        // skip 'run' zeros
        k += run; 

        if (k < 64 && k >= 0) {
            // find corresponding 2D coordinates
            int idx = zigzag_order[k];
            int u = idx / 8; // row
            int v = idx % 8;
            q_block[u][v] = (int16_t)val;
            
            // move to next position
            k++;
        } else {
            break; 
        }
    }
}

// Mode 3:
// Bit Reader Structure
// Used to read bits from file in both binary and ASCII modes
typedef struct {
    FILE *f;
    uint8_t buffer;  // bit buffer for binary mode
    int bits_left;   // number of bits left in buffer
    int is_binary;   // mode 
} BitReader;

// initialize BitReader
void init_bit_reader(BitReader *br, FILE *f, int is_binary) {
    br->f = f;
    br->buffer = 0;
    br->bits_left = 0; // no bits in buffer initially
    br->is_binary = is_binary;
}

// read a single bit from BitReader
int read_bit(BitReader *br) {
    if (br->is_binary) {
        // Binary mode 
        if (br->bits_left == 0) { // left 0
            // buffer empty, read next byte from file
            if (fread(&br->buffer, 1, 1, br->f) != 1) return -1; // EOF
            br->bits_left = 8; // refill 8 bits
        }
        // extract the leftmost (MSB) bit (left shift and mask)
        int bit = (br->buffer >> (br->bits_left - 1)) & 1;
        br->bits_left--;
        return bit;
    } else {
        // ASCII mode
        // only '0' and '1' are valid bits
        int c;
        while ((c = fgetc(br->f)) != EOF) {
            if (c == '0') return 0;
            if (c == '1') return 1;
        }
        return -1; // EOF
    }
}

// rebuild Huffman Tree from Codebook file

// new node allocation
HuffNode* new_node() {
    HuffNode *node = (HuffNode*)calloc(1, sizeof(HuffNode));
    return node;
}

// insert code into tree according to code string
// e.g. code_str="011", run=0, val=-1
void insert_code_to_tree(HuffNode *root, int run, int val, const char *code_str) {
    HuffNode *curr = root;
    for (int i = 0; code_str[i] != '\0'; i++) {
        if (code_str[i] == '0') {
            if (!curr->left) curr->left = new_node(); // 0 build left child
            curr = curr->left; // go left
        } else { // '1'
            if (!curr->right) curr->right = new_node(); // 1 build right child
            curr = curr->right; // go right
        }
    }
    // reached the end, this is a leaf
    curr->is_leaf = 1;
    curr->sym.run = run;
    curr->sym.val = val;
}

// rebuild Huffman Tree from Codebook file
HuffNode* rebuild_tree_from_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("[Error] Cannot open codebook: %s\n", filename);
        return NULL;
    }

    HuffNode *root = new_node(); // create root node
    int run, val;
    long count;
    char code_str[65536];

    // read each line: run val count code
    // e.g. 0 0 381024 1
    while (fscanf(f, "%d %d %ld %s", &run, &val, &count, code_str) == 4) {
        insert_code_to_tree(root, run, val, code_str);
    }
    fclose(f);
    return root;
}

// free memory of Huffman Tree
void free_tree_recursive(HuffNode *node) {
    if (!node) return;
    free_tree_recursive(node->left);
    free_tree_recursive(node->right);
    free(node);
}

// decode single symbol from BitReader
// read bits until reaching a leaf node
int decode_symbol(BitReader *br, HuffNode *root, int *run_out, int *val_out) {
    HuffNode *curr = root;
    while (curr && !curr->is_leaf) { // not leaf yet
        int bit = read_bit(br);
        if (bit == -1) return 0; // EOF or error
        
        if (bit == 0) curr = curr->left; // 0 go left, 1 go right
        else curr = curr->right;
    }
    // reached a leaf, return values
    if (curr && curr->is_leaf) {
        *run_out = curr->sym.run;
        *val_out = curr->sym.val;
        return 1; // success
    }
    return 0; // failure (code not found)
}

// decode entire 8x8 Block
void decode_huffman_block(BitReader *br, int16_t q_block[8][8], int16_t *prev_dc, 
                          HuffNode *dc_tree, HuffNode *ac_tree) {
    // 1. Clear block
    memset(q_block, 0, sizeof(int16_t) * 64);

    // 2. Decode DC (Diff)
    int run, val;
    if (!decode_symbol(br, dc_tree, &run, &val)) return; 
    
    // DPCM restore: current value = previous value + difference
    int16_t dc_val = *prev_dc + val;
    q_block[0][0] = dc_val; 
    *prev_dc = dc_val; // update prev_dc for next block

    // 3. Decode AC (Run-Length)
    int k = 1; // start from position 1 (0 is DC)
    while (k < 64) { 
        if (!decode_symbol(br, ac_tree, &run, &val)) break; // EOF or error

        // Check for EOB (End of Block): Run=0, Val=0
        if (run == 0 && val == 0) {
            break; // rest are all 0, no need to decode
        }

        k += run; // skip Run zeros
        if (k < 64) {
            int idx = zigzag_order[k]; // convert Zigzag to 2D index
            q_block[idx/8][idx%8] = (int16_t)val; // get set value
            k++;
        }
    }
}
//--------------------------------------------------------------
// Mode functions
int run_mode_0(int argc, char *argv[]) {
    // Mode 0 :
    // decoder 0 ResKimberly.bmp R.txt G.txt B.txt dim.txt
    if (argc < 7) {
        printf("Usage: decoder 0 <output.bmp> <R.txt> <G.txt> <B.txt> <dim.txt>\n");
        return 1;
    }


    const char *output_bmp = argv[2];
    const char *in_r = argv[3];
    const char *in_g = argv[4];
    const char *in_b = argv[5];
    const char *in_dim = argv[6];

    // 1. Read dimensions
    int width, height;
    if (!read_dim_txt(in_dim, &width, &height)) {
        printf("Error: Failed to read %s\n", in_dim);
        return 1;
    }

    printf("[Info] Decoding size %dx%d\n", width, height);

    // 2. Allocate memory
    Image *img = (Image*)malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->data_r = (uint8_t*)malloc(width * height);
    img->data_g = (uint8_t*)malloc(width * height);
    img->data_b = (uint8_t*)malloc(width * height);

    // 3. Read RGB data
    if (!read_channel_txt(in_r, img->data_r, width, height) ||
        !read_channel_txt(in_g, img->data_g, width, height) ||
        !read_channel_txt(in_b, img->data_b, width, height)) {
        printf("[Error] Failed to read RGB files.\n");
        free_image(img);
        return 1;
    }

    // 4. Write BMP file (rebuild Header and pixels)
    write_bmp(output_bmp, img);

    // 5. Cleanup
    free_image(img);
    
    printf("[Success] Decoder Mode 0 finished.\n");
    return 0;
}

// Mode 1(a): Dequantization + IDCT + YCbCr->RGB
int run_mode_1_a(int argc, char *argv[]) {
    // check argc count
    if (argc < 11) {
        printf("Usage: decoder 1 <QRes.bmp> <Orig.bmp> <QtY> <QtCb> <QtCr> <dim> <qFY> <qFCb> <qFCr>\n");
        return 1;
    }

    printf("[Info] Reading decoded data...\n");
    // 1. parse arguments
    const char *out_bmp = argv[2];
    const char *orig_bmp = argv[3];
    int qt_y[8][8], qt_cb[8][8], qt_cr[8][8]; // quantization tables
    read_qt_txt(argv[4], qt_y);
    read_qt_txt(argv[5], qt_cb);
    read_qt_txt(argv[6], qt_cr);

    int w, h; // image dimensions
    if (!read_dim_txt(argv[7], &w, &h)) return 1;

    FILE *f_qy = fopen(argv[8], "rb"); // open quantized files
    FILE *f_qcb = fopen(argv[9], "rb");
    FILE *f_qcr = fopen(argv[10], "rb");
    if(!f_qy || !f_qcb || !f_qcr) { printf("Error opening raw files\n"); return 1; }

    // 2. Prepare memory
    Image *recon_img = (Image*)malloc(sizeof(Image)); // reconstructed image
    recon_img->width = w; recon_img->height = h;
    recon_img->data_r = malloc(w*h); 
    recon_img->data_g = malloc(w*h); 
    recon_img->data_b = malloc(w*h);

    // 3. decoding loop
    int blocks_w = (w + 7) / 8; // number of 8x8 blocks
    int blocks_h = (h + 7) / 8;
    printf("[Info] Decoding Mode 1(a): %dx%d blocks\n", blocks_w, blocks_h);

    int16_t q_buf[8][8]; // quantized data buffer
    float dct_buf[8][8]; // temporary DCT buffer
    float spat_buf_y[8][8], spat_buf_cb[8][8], spat_buf_cr[8][8]; // IDCT output buffers

    // iterate all 8x8 blocks
    for (int by = 0; by < blocks_h; by++) { 
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }

        for (int bx = 0; bx < blocks_w; bx++) {
            
            // --- Y Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qy);      // read quantized data
            dequantize_8x8(q_buf, qt_y, dct_buf);         // dequantize F' = q * Q
            idct_8x8(dct_buf, spat_buf_y);                // IDCT

            // --- Cb Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qcb);
            dequantize_8x8(q_buf, qt_cb, dct_buf);
            idct_8x8(dct_buf, spat_buf_cb);

            // --- Cr Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qcr);
            dequantize_8x8(q_buf, qt_cr, dct_buf);
            idct_8x8(dct_buf, spat_buf_cr);

            // --- YCbCr to RGB and store to image ---
            for (int y = 0; y < 8; y++) { // pixel y in block
                for (int x = 0; x < 8; x++) { // pixel x in block
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    if (real_x >= w || real_y >= h) continue;

                    uint8_t r, g, b;
                    // convert YCbCr to RGB
                    ycbcr_to_rgb(spat_buf_y[y][x], spat_buf_cb[y][x], spat_buf_cr[y][x], &r, &g, &b);

                    int idx = real_y * w + real_x; // actual image index
                    recon_img->data_r[idx] = r; // store pixel
                    recon_img->data_g[idx] = g;
                    recon_img->data_b[idx] = b;
                }
            }
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    fclose(f_qy); fclose(f_qcb); fclose(f_qcr);
    printf("[Info] Writing output BMP...\n");
    // 4. write output BMP
    write_bmp(out_bmp, recon_img);

    // 5. calculate SQNR compared to original image
    Image *orig_img = read_bmp(orig_bmp);
    if (orig_img) {
        calculate_image_sqnr(orig_img, recon_img);
        free_image(orig_img);
    }

    free_image(recon_img);
    printf("[Success] Decoder Mode 1(a) finished.\n");
    return 0;
}

// Mode 1(b): with Error Compensation
int run_mode_1_b(int argc, char *argv[]) {
    // check argc count
    if (argc < 13) {
        printf("Usage: decoder 1 <Res.bmp> <QtY> <QtCb> <QtCr> <dim> <qFY> <qFCb> <qFCr> <eFY> <eFCb> <eFCr>\n");
        return 1;
    }

    // 1. parse arguments
    const char *out_bmp = argv[2];
    int qt_y[8][8], qt_cb[8][8], qt_cr[8][8];
    read_qt_txt(argv[3], qt_y);
    read_qt_txt(argv[4], qt_cb);
    read_qt_txt(argv[5], qt_cr);

    int w, h; // image dimensions
    if (!read_dim_txt(argv[6], &w, &h)) return 1;

    // open Quantized files
    FILE *f_qy = fopen(argv[7], "rb");
    FILE *f_qcb = fopen(argv[8], "rb");
    FILE *f_qcr = fopen(argv[9], "rb");
    // open Error files
    FILE *f_ey = fopen(argv[10], "rb");
    FILE *f_ecb = fopen(argv[11], "rb");
    FILE *f_ecr = fopen(argv[12], "rb");

    if(!f_qy || !f_qcb || !f_qcr || !f_ey || !f_ecb || !f_ecr) {
        printf("Error opening raw files\n"); return 1;
    }

    // 2. Prepare memory
    Image *recon_img = (Image*)malloc(sizeof(Image));
    recon_img->width = w; recon_img->height = h;
    recon_img->data_r = malloc(w*h); 
    recon_img->data_g = malloc(w*h); 
    recon_img->data_b = malloc(w*h);

    // 3. decoding loop
    int blocks_w = (w + 7) / 8; // number of 8x8 blocks
    int blocks_h = (h + 7) / 8;
    printf("[Info] Decoding Mode 1(b): %dx%d blocks\n", blocks_w, blocks_h);

    int16_t q_buf[8][8]; // quantized data buffer
    float err_buf[8][8]; // error data buffer
    float dct_buf[8][8]; // temporary DCT buffer
    float spat_buf_y[8][8], spat_buf_cb[8][8], spat_buf_cr[8][8]; // IDCT output buffers

    // iterate all 8x8 blocks
    for (int by = 0; by < blocks_h; by++) {
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }

        for (int bx = 0; bx < blocks_w; bx++) {
            
            // --- Y Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qy);
            fread(err_buf, sizeof(float), 64, f_ey);
            dequantize_8x8(q_buf, qt_y, dct_buf); // compute F' = q * Q
            // Key difference: add error back! F = F' + Error
            for(int u=0; u<8; u++) for(int v=0; v<8; v++) dct_buf[u][v] += err_buf[u][v];
            idct_8x8(dct_buf, spat_buf_y); // IDCT

            // --- Cb Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qcb);
            fread(err_buf, sizeof(float), 64, f_ecb);
            dequantize_8x8(q_buf, qt_cb, dct_buf);
            for(int u=0; u<8; u++) for(int v=0; v<8; v++) dct_buf[u][v] += err_buf[u][v];
            idct_8x8(dct_buf, spat_buf_cb);

            // --- Cr Channel ---
            fread(q_buf, sizeof(int16_t), 64, f_qcr);
            fread(err_buf, sizeof(float), 64, f_ecr);
            dequantize_8x8(q_buf, qt_cr, dct_buf);
            for(int u=0; u<8; u++) for(int v=0; v<8; v++) dct_buf[u][v] += err_buf[u][v];
            idct_8x8(dct_buf, spat_buf_cr);

            // --- YCbCr -> RGB ---
            for (int y = 0; y < 8; y++) { // pixel y in block
                for (int x = 0; x < 8; x++) { // pixel x in block
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    if (real_x >= w || real_y >= h) continue;

                    // convert YCbCr to RGB
                    uint8_t r, g, b;
                    ycbcr_to_rgb(spat_buf_y[y][x], spat_buf_cb[y][x], spat_buf_cr[y][x], &r, &g, &b);

                    int idx = real_y * w + real_x; // actual image index
                    recon_img->data_r[idx] = r;
                    recon_img->data_g[idx] = g;
                    recon_img->data_b[idx] = b;
                }
            }
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    fclose(f_qy); fclose(f_qcb); fclose(f_qcr);
    fclose(f_ey); fclose(f_ecb); fclose(f_ecr);

    printf("[Info] Writing output BMP...\n");
    // 4. write output BMP
    write_bmp(out_bmp, recon_img);
    free_image(recon_img);
    printf("[Success] Decoder Mode 1(b) finished.\n");
    return 0;
}

int run_mode_2(int argc, char *argv[]) {
    // decoder 2 <output.bmp> <ascii/binary> <input_code>
    if (argc < 5) {
        printf("Usage: decoder 2 <output.bmp> <ascii/binary> <input_code>\n");
        return 1;
    }

    // 1. parse arguments
    const char *out_bmp = argv[2];
    const char *format_str = argv[3];
    const char *in_code = argv[4];
    int is_binary = 0;

    // determine format
    if (strcmp(format_str, "binary") == 0) is_binary = 1;
    else if (strcmp(format_str, "ascii") == 0) is_binary = 0;
    else { printf("Unknown format: %s\n", format_str); return 1; }

    printf("[Info] Reading encoded data from %s (%s mode)...\n", in_code, format_str);
    
    // open input file
    FILE *f_in = fopen(in_code, is_binary ? "rb" : "r");
    if (!f_in) { printf("Error opening input file %s\n", in_code); return 1; }

    // 1. read image dimensions 
    int w, h;
    if (is_binary) {
        if (fread(&w, sizeof(int), 1, f_in) != 1) return 1;
        if (fread(&h, sizeof(int), 1, f_in) != 1) return 1;
    } else {
        if (fscanf(f_in, "%d %d", &w, &h) != 2) return 1;
    }
    printf("[Info] Decoding Mode 2 (%s): Size %dx%d\n", format_str, w, h);

    // allocate memory for reconstructed image
    Image *recon_img = (Image*)malloc(sizeof(Image));
    recon_img->width = w; recon_img->height = h;
    recon_img->data_r = malloc(w*h);
    recon_img->data_g = malloc(w*h);
    recon_img->data_b = malloc(w*h);

    int blocks_w = (w + 7) / 8; // number of 8x8 blocks
    int blocks_h = (h + 7) / 8;

    // temporary buffers
    int16_t q_y[8][8], q_cb[8][8], q_cr[8][8];
    float dct_buf[8][8];
    float spat_buf_y[8][8], spat_buf_cb[8][8], spat_buf_cr[8][8];

    // 2. decoding loop
    for (int by = 0; by < blocks_h; by++) {
        // progress display
        if (by % 10 == 0) {
            printf("\r[Info] Processing: %d%% (%d/%d rows)", (by * 100) / blocks_h, by, blocks_h);
            fflush(stdout); 
        }

        for (int bx = 0; bx < blocks_w; bx++) {
            
            // --- Y Channel ---
            decode_block_rle(f_in, q_y, is_binary);      // RLE -> Quantized
            dequantize_8x8(q_y, std_lum_qt, dct_buf);    // Dequantize (std table)
            idct_8x8(dct_buf, spat_buf_y);               // IDCT

            // --- Cb Channel ---
            decode_block_rle(f_in, q_cb, is_binary);
            dequantize_8x8(q_cb, std_chr_qt, dct_buf);   // Cb/Cr table
            idct_8x8(dct_buf, spat_buf_cb);

            // --- Cr Channel ---
            decode_block_rle(f_in, q_cr, is_binary);
            dequantize_8x8(q_cr, std_chr_qt, dct_buf);
            idct_8x8(dct_buf, spat_buf_cr);

            // --- YCbCr -> RGB ---
            for (int y = 0; y < 8; y++) { // pixel y in block
                for (int x = 0; x < 8; x++) { // pixel x in block
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    if (real_x >= w || real_y >= h) continue;

                    uint8_t r, g, b; // ycbcr to rgb
                    ycbcr_to_rgb(spat_buf_y[y][x], spat_buf_cb[y][x], spat_buf_cr[y][x], &r, &g, &b);

                    int idx = real_y * w + real_x; // actual image index
                    recon_img->data_r[idx] = r;
                    recon_img->data_g[idx] = g;
                    recon_img->data_b[idx] = b;
                }
            }
        }
    }
    printf("\r[Info] Processing: 100%% (%d/%d rows)\n", blocks_h, blocks_h);

    fclose(f_in);
    write_bmp(out_bmp, recon_img);
    free_image(recon_img);
    printf("[Success] Decoder Mode 2 finished.\n");
    return 0;
}

// Mode 3: Huffman Decoding
int run_mode_3(int argc, char *argv[]) {
    // Usage: decoder 3 <out.bmp> <ascii/binary> <code_prefix> <huffman_in>
    if (argc < 6) {
        printf("Usage: decoder 3 <out.bmp> <ascii/binary> <code_prefix> <huffman_in>\n");
        return 1;
    }

    const char *out_bmp = argv[2];
    const char *format_str = argv[3];
    const char *code_prefix = argv[4]; // codebook prefix
    const char *input_file = argv[5];
    int is_binary = (strcmp(format_str, "binary") == 0);
    printf("[Info] Mode 3 Decoding (%s)...\n", format_str);

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

    // 1. Rebuild 4 Huffman Trees
    char filename[256];
    
    snprintf(filename, 256, "%s_Y_DC.txt", code_prefix); // get filenames
    HuffNode *tree_y_dc = rebuild_tree_from_file(filename);
    
    snprintf(filename, 256, "%s_Y_AC.txt", code_prefix);
    HuffNode *tree_y_ac = rebuild_tree_from_file(filename);
    
    snprintf(filename, 256, "%s_C_DC.txt", code_prefix);
    HuffNode *tree_c_dc = rebuild_tree_from_file(filename);
    
    snprintf(filename, 256, "%s_C_AC.txt", code_prefix);
    HuffNode *tree_c_ac = rebuild_tree_from_file(filename);

    if (!tree_y_dc || !tree_y_ac || !tree_c_dc || !tree_c_ac) {
        printf("[Error] Failed to load codebooks with prefix '%s'\n", code_prefix);
        return 1;
    }

    // 2. Open input file
    FILE *f_in = fopen(input_file, is_binary ? "rb" : "r");
    if (!f_in) { printf("Error opening input %s\n", input_file); return 1; }

    // Initialize Bit Reader
    BitReader br;
    init_bit_reader(&br, f_in, is_binary);

    // 3. Read image width and height
    int w, h;
    if (is_binary) {
        fread(&w, sizeof(int), 1, f_in);
        fread(&h, sizeof(int), 1, f_in);
    } else {
        fscanf(f_in, "%d %d", &w, &h);
    }
    printf("[Info] Image Size: %dx%d\n", w, h);

    // 4. Prepare memory for reconstructed image
    Image *recon_img = (Image*)malloc(sizeof(Image));
    recon_img->width = w; recon_img->height = h;
    recon_img->data_r = malloc(w*h);
    recon_img->data_g = malloc(w*h);
    recon_img->data_b = malloc(w*h);

    int blocks_w = (w + 7) / 8;
    int blocks_h = (h + 7) / 8;

    // temporary buffers
    int16_t q_y[8][8], q_cb[8][8], q_cr[8][8];
    float dct[8][8], spat_y[8][8], spat_cb[8][8], spat_cr[8][8];
    int16_t prev_dc_y=0, prev_dc_cb=0, prev_dc_cr=0; // DPCM prediction initialized to zero

    // 4. Decoding loop
    for (int by = 0; by < blocks_h; by++) {
        if (by % 10 == 0) { // Display progress
            printf("\r[Info] Processing: %d%%", (by * 100) / blocks_h);
            fflush(stdout); 
        }
        for (int bx = 0; bx < blocks_w; bx++) {
            // Decode Y Channel
            // Read bits -> restore Quantized Block
            decode_huffman_block(&br, q_y, &prev_dc_y, tree_y_dc, tree_y_ac);
            // Dequantize + IDCT
            dequantize_8x8(q_y, current_lum_qt, dct);
            idct_8x8(dct, spat_y);

            // Decode Cb Channel
            decode_huffman_block(&br, q_cb, &prev_dc_cb, tree_c_dc, tree_c_ac);
            dequantize_8x8(q_cb, current_chr_qt, dct);
            idct_8x8(dct, spat_cb);

            // Decode Cr Channel
            decode_huffman_block(&br, q_cr, &prev_dc_cr, tree_c_dc, tree_c_ac);
            dequantize_8x8(q_cr, current_chr_qt, dct);
            idct_8x8(dct, spat_cr);

            // Convert back to RGB and fill image data
            for (int y = 0; y < 8; y++) { // pixel y in block
                for (int x = 0; x < 8; x++) { // pixel x in block
                    int real_x = bx * 8 + x; // actual image x
                    int real_y = by * 8 + y; // actual image y
                    if (real_x >= w || real_y >= h) continue;

                    uint8_t r, g, b;
                    ycbcr_to_rgb(spat_y[y][x], spat_cb[y][x], spat_cr[y][x], &r, &g, &b);

                    int idx = real_y * w + real_x; // actual image index
                    recon_img->data_r[idx] = r;
                    recon_img->data_g[idx] = g;
                    recon_img->data_b[idx] = b;
                }
            }
        }
    }
    printf("\r[Info] Processing: 100%%\n");

    // 5. Cleanup: free memory and close files
    fclose(f_in);
    free_tree_recursive(tree_y_dc); 
    free_tree_recursive(tree_y_ac);
    free_tree_recursive(tree_c_dc); 
    free_tree_recursive(tree_c_ac);
    
    write_bmp(out_bmp, recon_img);
    free_image(recon_img);
    printf("[Success] Mode 3 Decoded to %s\n", out_bmp);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: decoder <mode> ...\n");
        return 1;
    }

    int mode = atoi(argv[1]); // string to int

    if (mode == 0) {
        return run_mode_0(argc, argv);
    }
    else if (mode == 1) {
        // Distinguish between 1(a) and 1(b) by argument count
        if (argc == 11) {
            return run_mode_1_a(argc, argv);
        } else if (argc == 13) {
            return run_mode_1_b(argc, argv);
        } else {
            printf("Usage for 1(a): decoder 1 <Out.bmp> <Orig.bmp> ... (total 11 args)\n");
            printf("Usage for 1(b): decoder 1 <Out.bmp> ... (total 13 args)\n");
            return 1;
        }
    }
    else if (mode == 2) {
        return run_mode_2(argc, argv);
    }
    else if (mode == 3) {
        return run_mode_3(argc, argv);
    }
    else {
        printf("Mode %d not implemented yet.\n", mode);
    }

    return 0;
}