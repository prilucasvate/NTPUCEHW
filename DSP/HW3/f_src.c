#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define PI 3.14159265359

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

// filter function
// P = 1025, wc = PI/441 ( min(pi/L,pi/M) ) 
void LPF(double *h, int P, double wc) {
    int tau = (P - 1) / 2; // center index
    for (int n = 0; n < P; n++) {
        double sinc_val;
        // if n = tau, sinc(0) = wc/PI (L'Hopital's rule)
        if (n == tau) {
            sinc_val = wc / PI; // sinc / 0 special case
        } else {
            double x = n - tau; // shift for sinc 
            sinc_val = sin(wc * x) / (PI * x); // sinc function
        }
        
        // Hamming Window
        double window = 0.54 - 0.46 * cos(2 * PI * n / (P - 1));
        
        // combine sinc and window
        h[n] = sinc_val * window;
    }
}


int main(){
    // Open input WAV file
    FILE *fp = fopen("blue_giant_fragment_44.4kHz_16bits_stereo.wav", "rb");
    if (!fp) {
        printf("Failed to open input.wav\n");
        return 1;
    }

    // Read WAV header to struct
    WavHeader header;
    fread(&header, sizeof(WavHeader), 1, fp);

    printf("Input %dHz\n",header.sampleRate); // Print sample rate

    // Read PCM data to input_data
    short *input_data = (short *)malloc(header.subChunk2Size);
    fread(input_data, header.subChunk2Size, 1, fp); // PCM data
    fclose(fp);

    
    // Downsample factor L=80 M=441
    int in_samples = header.subChunk2Size / (header.numChannels * (header.bitsPerSample / 8)); // number of input samples
    int out_samples = in_samples * 80 / 441; // number of output samples
    short *output_data = (short *)malloc(out_samples * 2 * sizeof(short)); // output buffer
    
    // Design Low Pass Filter
    int P = 1025;
    int L = 80; // upsample factor
    int M = 441; // downsample factor
    double wc = fmin(PI/L, PI/M); // LPF cutoff frequency
    double *h = (double *)malloc(P * sizeof(double)); // filter coefficients 1025
    LPF(h, P, wc); // compute filter coefficients

    // --- Downsample with filtering ---
    printf("Start processing...\n");
    // --- start timing ---
    clock_t start = clock();
    
    for(int m=0 ; m < out_samples ; m++){ // each output sample
        double sum_left = 0.0;
        double sum_right = 0.0;

        // --- optimize caculation ---
        
        // 0 <= ( m*M - k*L + 512 ) < 1025

        // Center : m*M - k*L = 0
        // k around (m * M) / L 
        long long current_pos = (long long)m * M;
        
        //  P=1025, L=80, 1025/80 ~= 12.8
        // choose k from (current_pos / L - 15) to (current_pos / L + 15)
        int k_center = current_pos / L;
        int k_start = k_center - 15; 
        int k_end   = k_center + 15;

        // boundary check
        if (k_start < 0) k_start = 0;
        if (k_end >= in_samples) k_end = in_samples;

        for(int k = k_start ; k < k_end ; k++){ // each input sample
            int index = m * M - k * L + (P - 1) / 2; // filter index adjustment (centered) 0, 512 ,1024
            if(index >= 0 && index < P){ // check bounds, out of range (1025) -> 0
                // in LPF range, sum the contributions
                sum_left += h[index] * input_data[2 * k]; // left channel
                sum_right += h[index] * input_data[2 * k + 1]; // right channel
            }
        }
        
        // multiply 80 (energy conservation)
        sum_left *= L;  
        sum_right *= L; 

        // save to output 
        output_data[2 * m] = (short)sum_left;     // left channel
        output_data[2 * m + 1] = (short)sum_right; // right channel
    }
    // --- end timing ---
    clock_t end = clock();

    // end timing calculation
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Processing time: %.4f seconds\n", time_spent);

    // Write output WAV file
    FILE *fp_out = fopen("faster_output.wav", "wb");

    // fix header for output file (other fields remain the same)
    header.sampleRate = 8000;
    header.byteRate = 8000 * header.numChannels * header.bitsPerSample / 8;
    header.subChunk2Size = out_samples * header.numChannels * header.bitsPerSample / 8;
    header.chunkSize = header.subChunk2Size + 36; // 36 + subChunk2Size = total file size - 8

    // write header
    fwrite(&header, sizeof(WavHeader), 1, fp_out);

    // write PCM data
    fwrite(output_data, header.subChunk2Size, 1, fp_out);

    fclose(fp_out);

    // Free allocated memory
    free(input_data);
    free(output_data);
    free(h);
}