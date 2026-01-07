#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846

// 1. Hamming window function
// w[n] = 0.54 - 0.46 * cos(2*pi*n / (M-1))
double hamming(int n, int M) {
    if (M <= 1) return 1.0;
    return 0.54 - 0.46 * cos(2.0 * PI * n / (M - 1));
}

// 2. Compute DFT and convert to dB
// We only need Magnitude, not Phase
void compute_dft_db(double *signal, int N, double *output_db) {
    // Only compute 0 ~ N/2 (positive frequency part)
    for (int k = 0; k <= N / 2; k++) {
        double real = 0.0;
        double imag = 0.0;
        
        // DFT : X[k] = sum(x[n] * exp(-j * 2*pi * n * k / N))
        // Euler: exp(-ix) = cos(x) - i*sin(x)
        for (int n = 0; n < N; n++) {
            double angle = 2.0 * PI * n * k / N;
            real += signal[n] * cos(angle);
            imag += signal[n] * -sin(angle);
        }

        // Magnitude = sqrt(real^2 + imag^2)
        double mag = sqrt(real * real + imag * imag); // magnitude

        // dB: 20 * log10(mag)
        // Avoid log(0) by adding a small epsilon
        if (mag < 1e-15) mag = 1e-15;
        output_db[k] = 20.0 * log10(mag);
    }
}



// w_size: analysis window size (unit: millisecond)
// w_type: a string to be “hamming” or “rectangular”
// dft_size: DFT/FFT window size (unit: millisecond)
// f_itv: frame interval (unit: millisecond)
// wav_in: nput WAVE file
// spec_out: output spectrigram data (ascii with 15 decimal places)
int main(int argc, char *argv[]) {
    // check arguments
    if (argc != 7) {
        printf("Usage: %s w_size w_type dft_size f_itv wav_in spec_out\n", argv[0]);
        return 1;
    }

    // 1.read command line arguments
    double w_size = atof(argv[1]);     // Analysis window size (ms)
    char *w_type = argv[2];            // "hamming" or "rectangular"
    double dft_size = atof(argv[3]);   // DFT window size (ms)
    double f_itv = atof(argv[4]);      // Frame interval (ms)
    char *wav_in = argv[5];            // input WAVE file
    char *spec_out = argv[6];          // output spectrogram data (ascii)


    // 2. 開啟 WAV 檔並讀取 Sample Rate
    FILE *fp_in = fopen(wav_in, "rb");
    if (!fp_in) {
        fprintf(stderr, "Error: Cannot open input file %s\n", wav_in);
        return 1;
    }

    // --- WAV Header Info ---
    // get sample rate
    unsigned int sample_rate = 0;
    fseek(fp_in, 24, SEEK_SET); // move to sample rate field (24th byte)
    fread(&sample_rate, sizeof(unsigned int), 1, fp_in);
    
    // move file pointer to the beginning of data section (44th byte)
    fseek(fp_in, 44, SEEK_SET);

    // move to the end to calculate total samples
    long data_start_pos = 44;
    fseek(fp_in, 0, SEEK_END);
    long file_size = ftell(fp_in);
    long total_samples = (file_size - data_start_pos) / 2; // (bits per sample) 16-bit audio = 2 bytes per sample
    fseek(fp_in, data_start_pos, SEEK_SET); // reset to data start position

    // 3. change ms to samples
    // samples = ms * (sample_rate / 1000)
    int P = (int)(w_size * sample_rate / 1000.0);   // Analysis Window Size
    int N = (int)(dft_size * sample_rate / 1000.0); // DFT Size to use
    int Q = (int)(f_itv * sample_rate / 1000.0);    // DFT Shift Size

    // Fs/N = frequency resolution : the clearer the frequency, the larger N is. (like PCM sampling theorem)
    // Fs/Q = time resolution : the clearer the time, the smaller Q is
    // P should be <= N, P is the actual window size, N is the zero-padded size for DFT calculation
    // Trade-off: large P -> good frequency resolution, poor time resolution


    // 4. determine window type 
    // use_hamming = 1 --> hamming window
    // use_hamming = 0 --> rectangular window
    int use_hamming = 0;
    if (strcmp(w_type, "hamming") == 0) use_hamming = 1;
    else if (strcmp(w_type, "rectangular") == 0) use_hamming = 0;
    else {
        fprintf(stderr, "Error: Unknown window type '%s'. Use 'hamming' or 'rectangular'.\n", w_type);
        fclose(fp_in);
        return 1;
    }

    printf("Processing %s:\n", wav_in);
    printf("  Sample Rate: %d Hz\n", sample_rate);
    printf("  P (Analysis): %d samples (%.2f ms)\n", P, w_size);
    printf("  N (DFT Size): %d samples (%.2f ms)\n", N, dft_size);
    printf("  Q (Shift):    %d samples (%.2f ms)\n", Q, f_itv);
    printf("  Window Type: %s\n", use_hamming ? "Hamming" : "Rectangular");

    // 5. prepare memory
    short *input_buffer = (short *)malloc(total_samples * sizeof(short)); // whole input signal
    double *frame_buffer = (double *)malloc(N * sizeof(double)); // DFT input frame
    double *output_db = (double *)malloc((N/2 + 1) * sizeof(double)); // DFT output in dB (N/2+1 points for real input)

    if (!input_buffer || !frame_buffer || !output_db) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // 6. read input signal
    fread(input_buffer, sizeof(short), total_samples, fp_in); // read all samples
    fclose(fp_in); 

    // 7. prepare output file
    FILE *fp_out = fopen(spec_out, "w");
    if (!fp_out) {
        fprintf(stderr, "Error: Cannot open output file %s\n", spec_out);
        return 1;
    }

    // 8. main processing loop ()STFT)
    // move frame by Q samples until the end 
    for (int start_idx = 0; start_idx <= total_samples - P; start_idx += Q) {
        // 5.1 initialize frame buffer
        // set all N points to 0.0 (for zero-padding if P < N)
        for (int i = 0; i < N; i++) {
            frame_buffer[i] = 0.0;
        }

        // 5.2 load P samples and apply window
        for (int i = 0; i < P; i++) {
            // get sample value
            double val = (double)input_buffer[start_idx + i];
            
            // apply hamming window
            if (use_hamming) {
                val *= hamming(i, P);
            }
            // Rectangular is just val *= 1.0, so no need to do anything
            frame_buffer[i] = val; // store in frame buffer
        }
        // Note: If P < N, the remaining frame_buffer[P...N-1] are still 0, which is zero-padding!

        // 5.3 compute DFT and convert to dB
        compute_dft_db(frame_buffer, N, output_db);

        // 5.4 output dB values to file
        // Format: each line is the spectrum of one frame (0 ~ N/2)
        for (int k = 0; k <= N / 2; k++) {
            fprintf(fp_out, "%.15f ", output_db[k]);
        }
        fprintf(fp_out, "\n");
    }

    // 9. cleanup
    printf("Generated spectrogram data to %s\n", spec_out);
    
    free(input_buffer);
    free(frame_buffer);
    free(output_db);
    fclose(fp_out);

    return 0;
}