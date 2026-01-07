#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define PI 3.14159265359

#define P 441      // Input Filter Size (each frame)
#define Q 1025     // Filter Size
#define N 2048     // FFT Size (N > = P + Q - 1)
#define L 80       // Upsampling factor 
#define M 441      // Downsampling factor

// Complex number structure
typedef struct {
    double real;
    double imag;
} Complex;

// WAV file header structure
typedef struct {
    char riff[4];           // "RIFF"
    int chunkSize;          // (Size of the whole file - 8) bytes
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    int subChunk1Size;      // 16 for PCM
    short audioFormat;      // 1 for PCM
    short numChannels;      // 2 for Stereo
    int sampleRate;         // 44100
    int byteRate;           // SampleRate * NumChannels * BitsPerSample/8 (bytes per second)
    short blockAlign;       // NumChannels * BitsPerSample/8 (bytes per sample)
    short bitsPerSample;    // 16 (bits per sample)
    char data[4];           // "data"
    int subChunk2Size;      // Data size
} WavHeader;

// Low Pass Filter using Sinc and Hamming Window
// len = 1025, wc = 2.0 * PI * 4000 / 44100
void LPF(Complex *h, int len, double wc) {
    int tau = (len - 1) / 2; // center index 512

    for (int n = 0; n < len ; n++) {
        // --- Sinc Function ---
        double h_val;
        if (n == tau) {
            h_val = wc / PI; // Sinc(0) special case
        } else {
            double k = n - tau; // shift to center
            h_val = sin(wc * k) / (PI * k); // Sinc function
        }

        // --- Hamming Window ---
        double win = 0.54 - 0.46 * cos(2 * PI * n / (len - 1));
        
        // Fill coefficients (real part only)
        h[n].real = h_val * win;
        h[n].imag = 0.0;
    }
}

// invert = 0 : FFT
// invert = 1 : IFFT
void fft(Complex *x, int n, int invert){
    // 1. Bit-reversal Permutation
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (i < j) { // only need to swap half the time
            // Swap x[i] and x[j]
            Complex temp = x[i]; 
            x[i] = x[j]; 
            x[j] = temp;
        }
        int m = n >> 1; // check from highest bit (MSB)
        while (m >= 1 && j >= m) { // while j's MSB is 1
            j -= m; // clear that bit
            m >>= 1; // move to next lower bit (like carry)
        }
        j += m; // set that bit to 1 (like increment)
    }

    // 2. Butterfly Operations
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? 1 : -1); // FFT * -1, IFFT * 1
        Complex wlen = {cos(ang), sin(ang)}; // Wlen = e^(i*ang)
        
        for (int i = 0; i < n; i += len) {
            Complex w = {1.0, 0.0}; // reset w for each segment
            for (int j = 0; j < len / 2; j++) { // seperate half
                Complex u = x[i + j];  // top part (even index)
                Complex v; // bottom part (odd index)

                // Complex multiplication: v = x[i+j+len/2] * w (odd index)
                v.real = x[i + j + len/2].real * w.real - x[i + j + len/2].imag * w.imag;
                v.imag = x[i + j + len/2].real * w.imag + x[i + j + len/2].imag * w.real;

                // Combine even and odd parts
                x[i + j].real = u.real + v.real; // top part (even index)
                x[i + j].imag = u.imag + v.imag;
                x[i + j + len/2].real = u.real - v.real; // bottom part (odd index)
                x[i + j + len/2].imag = u.imag - v.imag;

                // Update w = w * Wlen
                double temp_r = w.real * wlen.real - w.imag * wlen.imag;
                w.imag = w.real * wlen.imag + w.imag * wlen.real;
                w.real = temp_r;
            }
        }
    }
}

int main(){
    // Open input WAV file
    // blue_giant_fragment_44.4kHz_16bits_stereo
    FILE *fp_in = fopen("blue_giant_fragment_44.4kHz_16bits_stereo.wav", "rb");
    if (!fp_in) {
        printf("Failed to open input.wav\n");
        return 1;
    }
    FILE *fp_out = fopen("output2.wav", "wb");

    // Read WAV header to struct
    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, fp_in);

    // calculate number of samples
    int total_samples = header.subChunk2Size / (header.numChannels * sizeof(short));
    printf("Input: %d Hz, %d samples\n", header.sampleRate, total_samples);

    // fix header for output file (other fields remain the same)
    header.sampleRate = 8000;
    header.byteRate = 8000 * header.numChannels * sizeof(short);
    // output samples
    int out_samples_total = (long long)total_samples * 80 / 441;
    header.subChunk2Size = out_samples_total * header.numChannels * sizeof(short);
    header.chunkSize = header.subChunk2Size + 36;
    fwrite(&header, sizeof(WavHeader), 1, fp_out);

    // allocate memory 
    short *input_buf = (short*)malloc(P * 2 * sizeof(short)); // stereo input buffer
    // overlap buffers for left and right channels
    double *overlap_L = (double*)calloc(N, sizeof(double)); 
    double *overlap_R = (double*)calloc(N, sizeof(double));
    // FFT buffers
    Complex *H = (Complex*)malloc(N * sizeof(Complex)); // filter frequency response
    Complex *X = (Complex*)malloc(N * sizeof(Complex)); // input frame FFT

    // low-pass filter
    double wc = 2.0 * PI * 4000 / 44100; // cutoff frequency 44100Hz -> 4000Hz
    LPF(H, Q, wc);
    fft(H, N, 0); // FFT h[n] to H[k]

    clock_t start_time = clock(); // start timing

    // --------------------------------
    int read_count;
    // output buffer for 80 samples (stereo), from 441 down to 80
    short *out_buf = (short*)malloc(80 * 2 * sizeof(short)); 

    while ((read_count = fread(input_buf, sizeof(short), P * 2, fp_in)) > 0) {
        int samples_read = read_count / 2; // number of stereo samples read

        // do FFT-based filtering for both channels
        
        // clean time-domain segments after filtering
        double clean_segment[2][P]; 

        // ch = 0 : Left, ch = 1 : Right
        for (int ch = 0; ch < 2; ch++) {
            // prepare input frame X[n]
            for (int i = 0; i < N; i++) {
                if (i < samples_read) {
                    X[i].real = (double)input_buf[2 * i + ch]; // read L or R
                    X[i].imag = 0.0;
                } else { // zero padding
                    X[i].real = 0.0; 
                    X[i].imag = 0.0;
                }
            }
            // X[] = [1, 2, 3, ..., P, 0, 0, ..., 0] length N

            // FFT transform X[n] -> X[k]
            fft(X, N, 0);

            // Frequency domain filtering X[k] = X[k] * H[k]
            for (int i = 0; i < N; i++) {
                // complex multiplication
                double temp_r = X[i].real * H[i].real - X[i].imag * H[i].imag;
                double temp_i = X[i].real * H[i].imag + X[i].imag * H[i].real;
                X[i].real = temp_r; 
                X[i].imag = temp_i;
            }

            // IFFT transform 
            fft(X, N, 1);

            // Overlap-Add processing
            // choose overlap buffer L or R
            double *current_overlap = (ch == 0) ? overlap_L : overlap_R;
            
            // --- handle clean output segment (0 to P-1) ---
            for (int i = 0; i < P; i++) {
                // 1. Normalization + add the tail from previous round
                double val = (X[i].real / N) + current_overlap[i];
                
                // 2. Save the valid output (front 441 samples)
                clean_segment[ch][i] = val;
            }
            
            // --- handle overlap for next round (P to N-1) ---
            // Update overlap buffer for next round (2048 - 441 = 1607 samples)
            for (int k = 0; k < N - P; k++) {
                // original i = k + P
                int i = k + P; 
                
                // Calculate the new tail (X[i]) + the old tail from previous round
                double val = (X[i].real / N) + current_overlap[i];
                
                // Store at the beginning of the buffer
                current_overlap[k] = val;
            }

            // --- Clear the remaining buffer ---
            // zero out the rest of the overlap buffer
            for (int k = N - P; k < N; k++) {
                current_overlap[k] = 0.0;
            }
        }

        // Resample from 441 to 80
        
        for (int m = 0; m < 80; m++) {
            // get output sample position originally from 441 samples
            double pos = m * (double)441 / 80; // e.g. 0, 5.5125, 11.025, ...
            int idx = (int)pos; // floor e.g. 0, 5, 11, ...
            double frac = pos - idx; // distance to left index e.g. 0.0, 0.5125, 0.025, ...

            if (idx >= samples_read - 1) idx = samples_read - 2; // boundary protection
            if (idx < 0) idx = 0;

            // linear interpolation
            // val = left val * (1 - frac) + right val * frac
            double val_L = clean_segment[0][idx] * (1.0 - frac) + clean_segment[0][idx+1] * frac;
            double val_R = clean_segment[1][idx] * (1.0 - frac) + clean_segment[1][idx+1] * frac;

            // save to output buffer
            out_buf[2 * m]     = (short)val_L;
            out_buf[2 * m + 1] = (short)val_R;
        }

        fwrite(out_buf, sizeof(short), 80 * 2, fp_out);
    }

    // timing end
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Processing done. Time: %.4f seconds\n", time_spent);

    // free memory 
    free(input_buf); free(out_buf);
    free(overlap_L); free(overlap_R);
    free(H); free(X);
    fclose(fp_in); fclose(fp_out);

    printf("Output saved to output.wav\n");
    return 0;
}