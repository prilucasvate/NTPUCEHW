#include <stdio.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>
#define PI 3.14159265359
FILE *fp;// a file point to save waveform
void add_header(FILE *fp,int N,int fs);
void sine_wav(FILE *fp,int fs, int f, double L);
int main(int argc, char * argv[]){
    if (argc != 5) {
        fprintf(stderr,"need :  %s fs f L out_fn\n",argv[0]);
        return 1;
    }
    int fs = atoi(argv[1]);
    int f =  atoi(argv[2]);
    double L = atof(argv[3]);
    char *out = argv[4];
    printf("fs:%d, f:%d, L:%lf, fn:%s ",fs,f,L,out);
    int N = L*fs;
    fp = fopen(out, "wb");
    if (!fp) { perror(out); return 1; }
    add_header(fp,N,fs);
    sine_wav(fp,fs,f,L);
}
void add_header(FILE *fp,int N,int fs){
    //short 16bit int 32bit
    int numSamples = N;
    short audioFormat = 1; //PCM =1 
    short numChannels = 2; //Stereo =2
    int sampleRate = fs;
    short bitsPerSample = 16; //16 bits
    short blockAlign = numChannels*bitsPerSample/8;
    int byteRate = sampleRate*numChannels*bitsPerSample/8;
    int subChunk2Size = numSamples*numChannels*bitsPerSample/8;
    int subChunk1Size = 16; //PCM
    int chunkSize = 4 + (8+subChunk1Size) + (8+subChunk2Size);
    //----------------
    fwrite("RIFF", sizeof(char), 4, fp);//0-3 big endian
    fwrite(&chunkSize,sizeof(chunkSize),1,fp);//4-7 
    fwrite("WAVE", sizeof(char), 4, fp);//8-11
    //------------------
    fwrite("fmt ", sizeof(char), 4, fp);//12-15
    fwrite(&subChunk1Size,sizeof(subChunk1Size),1,fp);//16-19
    fwrite(&audioFormat,sizeof(audioFormat),1,fp);//20-21
    fwrite(&numChannels,sizeof(numChannels),1,fp);//22-23
    fwrite(&sampleRate,sizeof(sampleRate),1,fp);//24-27
    fwrite(&byteRate,sizeof(byteRate),1,fp);//28-31
    fwrite(&blockAlign,sizeof(blockAlign),1,fp);//32-33
    fwrite(&bitsPerSample,sizeof(bitsPerSample),1,fp);//34-35
    //---------------------
    fwrite("data", sizeof(char), 4, fp);//36-39
    fwrite(&subChunk2Size,sizeof(subChunk2Size),1,fp);//40-43
    //->pcm
}
void sine_wav(FILE *fp,int fs, int f, double L){
    //fs:sampling frequency f:frequency of sin wave L:length of sin wave in an unit of second
    double T = 1.0/fs;// sampling period 1/16k
    double A = 32767;// amplitude of sin wave
    double tmp;// temp variable
    int N = L*fs;// length of sin wave (sample)
    int n=0;//sample index
    // out[]:file name
    for(n=0;n<N;n++){
        tmp=A*sin(2*PI*f*n*T); // x[t]
        short left=(short)floor(tmp+0.5); //rounding
        fwrite( &left, sizeof(left), 1, fp);// write the waveform to the file

        tmp=A*cos(2*PI*f*n*T);
        short right=(short)floor(tmp+0.5); //rounding
        fwrite( &right, sizeof(right), 1, fp);// write the waveform to the file
    }
    if( !fp ) {// chech if the file is opened sucessfully
        fprintf(stderr, "Cannot save \n");
        exit(1);// stop and exit this program if error
    }
    fclose(fp);
}