#include <stdio.h>
#include <stdlib.h>
#define PI 3.14159265359
short RCfilter(short in, double *prev, double alpha);

int main(int argc,char *argv[]){
    if (argc != 3) {
        fprintf(stderr, "Use: %s input.wav output.wav\n", argv[0]);
        return 1;
    }
    FILE *in_f, *out_f;
    in_f=fopen(argv[1],"rb");
    out_f=fopen(argv[2],"wb");
    //-------------------------------------------
    int sampleRate,subChunk2Size;
    short numChannels,bitsPerSample;
    fseek( in_f, 22, SEEK_SET );
    fread(&numChannels, 2, 1, in_f);
    fread(&sampleRate, 4, 1, in_f);
    fseek( in_f, 34, SEEK_SET );
    fread(&bitsPerSample, 2, 1, in_f);
    fseek( in_f, 40, SEEK_SET );
    fread(&subChunk2Size, 4, 1, in_f);
    //-----------------------------------------
    fseek(in_f, 0, SEEK_SET);
    char header[44]; //same header
    fread(header, 1, 44, in_f);
    fwrite(header, 1, 44, out_f); 
    //-----------------------------------------
    double RC = 1.0/(800*PI);
    double tau = 1.0/sampleRate;
    double alpha = (RC/(RC+tau));
    double prevL= 0.0, prevR= 0.0;
    int numSamples=subChunk2Size*8/bitsPerSample;

    short *pcm=(short*)malloc(sizeof(short)*numSamples);
    short *fpcm=(short*)malloc(sizeof(short)*numSamples);// filterd PCM
    fread(pcm, sizeof(short), numSamples, in_f); // PCM data 
    //warm up 7RC
    int warm = (int)((7*RC) * sampleRate);
    for (int n = 0; n < warm; ++n) {
        (void)RCfilter(pcm[2*n],     &prevL, alpha);
        (void)RCfilter(pcm[2*n + 1], &prevR, alpha);
    }
    //
    printf("warm samples: %d\n", warm);
    for(int i=0;i<numSamples;i+=2){
        fpcm[i]=RCfilter(pcm[i], &prevL,alpha);
        fpcm[i+1]=RCfilter(pcm[i+1], &prevR,alpha);
        printf("L:%7d R:%7d\n", fpcm[i], fpcm[i+1]);
    }

    fseek( out_f, 44, SEEK_SET );
    fwrite(fpcm, sizeof(short), numSamples, out_f);

    free(pcm); 
    free(fpcm);
    fclose(in_f); 
    fclose(out_f);
}

short RCfilter(short in, double *prev, double alpha){
    double out = alpha*(*prev) + (1-alpha)*in; // calculate output
    if (out > 32767.) out = 32767;
    else if (out < -32768) out = -32768;
    *prev = out;
    return (short)out;
}
