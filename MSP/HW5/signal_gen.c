#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI 3.141592653589793

/* ================= WAV HEADER ================= */

typedef struct {
    char riff[4];        // "RIFF"
    int  size;
    char wave[4];        // "WAVE"
    char fmt[4];         // "fmt "
    int  fmt_size;       // 16
    short format;        // 1 = PCM
    short channels;      // mono
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data[4];        // "data"
    int data_size;
} WavHeader;

/* ================= PARAMETERS ================= */

double amp[10] = {
    100, 2000, 1000, 500, 250,
    100, 2000, 1000, 500, 250
};

double freq[10] = {
    0, 31.25, 500, 2000, 4000,
    44, 220, 440, 1760, 3960
};

/* ================= WAVE GENERATION ================= */

double gen_wave(int type, double t, double f)
{
    if (f == 0) return 0.0;

    switch (type) {
        case 0: // sine
            return sin(2 * PI * f * t);

        case 1: // sawtooth
            return f * t - floor(f * t);

        case 2: // square
            return sin(2 * PI * f * t) >= 0 ? 1.0 : -1.0;

        case 3: // triangle
            return 2.0 * fabs(2.0 * (f * t - floor(f * t + 0.5))) - 1.0;
    }
    return 0.0;
}

/* ================= WRITE WAV ================= */

void write_wav(const char *filename, short *data, int nsamples, int fs)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    WavHeader h;
    memcpy(h.riff, "RIFF", 4);
    memcpy(h.wave, "WAVE", 4);
    memcpy(h.fmt,  "fmt ", 4);
    memcpy(h.data, "data", 4);

    h.fmt_size = 16;
    h.format = 1;
    h.channels = 1;
    h.sample_rate = fs;
    h.bits_per_sample = 16;
    h.byte_rate = fs * 2;
    h.block_align = 2;
    h.data_size = nsamples * 2;
    h.size = 36 + h.data_size;

    fwrite(&h, sizeof(WavHeader), 1, fp);
    fwrite(data, sizeof(short), nsamples, fp);
    fclose(fp);
}

/* ================= MAIN ================= */

int main(void)
{
    int fs_list[2] = {8000, 16000};

    for (int k = 0; k < 2; k++) {
        int fs = fs_list[k];
        int total_samples = fs * 4;   // 4 seconds total
        short *buffer = calloc(total_samples, sizeof(short));

        for (int j = 0; j < 4; j++) {          // waveform type
            for (int i = 0; i < 10; i++) {     // frequency index

                double t_start = j + 0.1 * i;
                double t_end   = j + 0.1 * (i + 1);

                int n_start = (int)(t_start * fs);
                int n_end   = (int)(t_end   * fs);

                for (int n = n_start; n < n_end; n++) {
                    double t = (double)n / fs;
                    double local_t = t - t_start;

                    double s = gen_wave(j, local_t, freq[i]);
                    buffer[n] += (short)(amp[i] * s);
                }
            }
        }

        char filename[32];
        sprintf(filename, "s-%dkHz.wav", fs / 1000);
        write_wav(filename, buffer, total_samples, fs);

        free(buffer);
        printf("Generated %s\n", filename);
    }

    return 0;
}