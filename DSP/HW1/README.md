# DSP HW1 
 Simulation of RC Low-Pass Filter by Discrete Signal Processing   
(you can use **./run.sh 8000 100 0.1** to run gen -> filter -> draw)  
(usage: ./run.sh fs f L it will generate 2 wav and 2 txt  files for check)
## Problem 1
### 題目
If $x(t) = e^{j\Omega t}$ ,find $y(t)$ .Note that $y(t)$ can be expressed by $y(t) = H(\Omega) e^{j \Omega t}$ where $H(\Omega)$ is a function of $\Omega$ with parameters of $R$ and $C$ .($H(\Omega )$ would be the Fourier Transform! By handwriting mathematics. Please keep the calculation process on your paper).
### 手寫
![Screenshot_20251007_164417_Samsung Notes](https://hackmd.io/_uploads/HkEM5vM6eg.jpg)

### 解題
#### 1.Homogenous
從Eq.(3) 得知 $x(t)=RC\frac{dy(t)}{dt}+y(t)$  
接著計算$RC\frac{dy(t)}{dt}+y(t)=0$  
$\frac{dy(t)}{dt}=-\frac{1}{RC}y(t)$  
$\frac{1}{y(t)}dy(t)=-\frac{1}{RC}dt$  
$ln|y(t)|=-\frac{t}{RC}+C$  
$y_h(t)=e^{-\frac{t}{RC}}\cdot e^C = A \cdot e^{-\frac{t}{RC}}$ (A is constant)  
#### 2.Particular
從Eq.(3) 得知 $x(t)=RC\frac{dy(t)}{dt}+y(t)$  
$\frac{dy(t)}{dt}=j\Omega \cdot H(\Omega)e^{j\Omega t}$  (replace in Eq.(3))  
$x(t)=RC\cdot j\Omega\cdot H(\Omega)e^{j\Omega t}+H(\Omega)e^{j\Omega t}$  
$e^{j\Omega t}=RCj\Omega\cdot H(\Omega)e^{j\Omega t}+H(\Omega)e^{j\Omega t}$ (消$e^{j\Omega t}$)  
$1=(RCj\Omega +1)H(\Omega)$  
$H(\Omega)=\frac{1}{j\Omega RC+1}$ (replace $y(t)=H(\Omega)e^{j\Omega t}$)  
$y_p(t)=\frac{1}{j\Omega RC+1}e^{j\Omega t}$
#### 3.General
$y(t)=y_h(t)+y_p(t)=Ae^{-\frac{t}{RC}}+\frac{1}{j\Omega RC+1}e^{j\Omega t}$ (A is constant)  
## Problem 2
### 題目
If $x(t)=e^{j\Omega t}u(t)$where $u(t)$ is the unit step function, find $y(t)$. Note that $y(t)$ can be expressed by a transient-state response and a steady-state response. ($H(\Omega)$ would be the Fourier Transform! By handwriting mathematics. Please keep the calculation process on your paper).
### 手寫
![image](https://hackmd.io/_uploads/SJuWJTfaxx.png)

### 解題
從Eq.(3) 得知 $x(t)=RC\frac{dy(t)}{dt}+y(t)$  
#### 1.Homogenous
$RC\cdot \frac{dy(t)}{dt}+y(t)=0$  
from problem 1 -> $y_h(t)= A \cdot e^{-\frac{t}{RC}}$ (A is constant)  
#### 2.Particular
$x(t)=RC\frac{dy(t)}{dt}+y(t)$  
$y_p(t)=H(\Omega)e^{j\Omega t}$  
$e^{j\Omega t}=RCj\Omega\cdot H(\Omega)e^{j\Omega t}+H(\Omega)e^{j\Omega t}$ (消$e^{j\Omega t}$)  
$H(\Omega)=\frac{1}{j\Omega RC+1}$ (replace $y(t)=H(\Omega)e^{j\Omega t}$)  
$y_p(t)=\frac{1}{j\Omega RC+1}e^{j\Omega t}$ 
#### 3.General
$y(t)=[y_h(t)+y_p(t)]u(t)=[Ae^{-\frac{t}{RC}}+\frac{1}{j\omega RC+1}e^{j\omega t}]u(t)$ (A is constant)  
#### 4.Initial condition
t=0, u(0)=1  
$y(0)=0=A+\frac{1}{j\Omega RC+1}$  
$A=-\frac{1}{j\Omega RC+1}$(replace in y(t))  
$y(t)=-\frac{1}{j\Omega RC+1}(e^{-\frac{t}{RC}}-e^{j\Omega t})u(t)$  

## Problem 3
### 題目
If $x(t) = e^{j\Omega t}$ , $R = 1000\Omega$ ,and $C = \left(\frac{1}{2\pi} \times \frac{1}{400} \times \frac{1}{1000}\right)$ , find $y(t)$ . for $\Omega = 2\pi \cdot f$ , $f = 100\text{Hz}, 400\text{Hz},$ and $3000\text{Hz}$ .(By handwriting mathematics. Please keep the calculation process on your paper).
### 手寫
![image](https://hackmd.io/_uploads/B1pWg0zTle.png)


### 解題
(from problem 1)  $y(t)=\frac{1}{j\Omega RC+1}e^{j\Omega t}$  
$RC=\frac{1}{800\pi}$
#### 1. 100Hz ($\Omega=2\pi f=200\pi$ )  
$y(t)=\frac{1}{j\cdot2\pi\times100\times\frac{1}{800\pi}+1}e^{j200\pi t}=\frac{1}{j0.25+1}e^{j200\pi t}$  
$H=\frac{1}{j\alpha+1}\frac{1-j\alpha}{1-j\alpha}=\frac{1-j\alpha}{1+\alpha^2}=\frac{1}{1+\alpha^2}+j\frac{-\alpha}{1+\alpha^2}$  
$|H|=\sqrt{\frac{1}{1+\alpha^2}+j\frac{-\alpha}{1+\alpha^2}}=\sqrt{\frac{(1+\alpha^2)}{(1+\alpha^2)^2}}=\sqrt{\frac{1}{(1+\alpha^2)}}=\frac{1}{\sqrt{1+\alpha^2}}$  
$\phi=tan^{-1}(\frac{\frac{-\alpha}{1+\alpha^2}}{\frac{1}{1+\alpha^2}})=tan^{-1}(-\alpha)$  
$|H(200\pi)|=\frac{1}{\sqrt{1+0.25^2}}\approx0.97$  
$\angle H(200\pi) = -\tan^{-1}(0.25)\approx-14.04^{\circ} \approx -0.245 \ (rad)$  
$y(t)=0.97e^{-j0.245} \cdot e^{j200\pi t} = 0.97 e^{j (200\pi t - 0.245)}$  
$Delay = \frac{-0.245}{200\pi} \approx -0.00039\ (s)$  
#### 2. 400Hz ($\Omega=2\pi f=800\pi$ )  
$y(t)=\frac{1}{j\cdot2\pi\times 400\times\frac{1}{800\pi}+1}e^{j800\pi t}=\frac{1}{j+1}e^{j800\pi t}$  
$|H(800\pi)|=\frac{1}{\sqrt{1+1^2}}\approx0.707$  
$\angle H(800\pi) = -\tan^{-1}(1)\approx -45^{\circ} \approx -0.785 \  (rad)$  
$y(t)=0.707e^{-j0.785} \cdot e^{j800\pi t} = 0.707 e^{j (800\pi t - 0.785)}$  
$Delay = \frac{-0.785}{800\pi} \approx -0.00031 (s)$
#### 3. 3000Hz ($\Omega=2\pi f=6000\pi$ )  
$y(t)=\frac{1}{j\cdot2\pi\times 3000\times\frac{1}{6000\pi}+1}e^{j6000\pi t}=\frac{1}{j7.5+1}e^{j6000\pi t}$  
$|H(6000\pi)|=\frac{1}{\sqrt{1+7.5^2}}\approx0.132$  
$\angle H(6000\pi) = -\tan^{1}(-7.5)\approx -1.438 \  (rad)$  
$y(t)=0.132e^{-j1.438} \cdot e^{j6000\pi t} = 0.132 e^{j (6000\pi t - 1.438)}$  
$Delay = \frac{-1.438}{6000\pi} \approx -0.000076\ (s)$
## Problem 4
### 題目
If  $x(t) = e^{j\Omega t}u(t)$ , $R = 1000\Omega$ ,and $C = \left(\frac{1}{2\pi} \times \frac{1}{400} \times \frac{1}{1000}\right)$ ,find $y(t)$ . for $\Omega = 2\pi \cdot f$ , $f = 100\text{Hz}, 400\text{Hz},$ and $3000\text{Hz}$ .(By handwriting mathematics. Please keep the calculation process on your paper). 
### 手寫
![Screenshot_20251007_164454_Samsung Notes](https://hackmd.io/_uploads/Bkk6ivz6xl.jpg)

### 解題
from Eq.3 $x(t)=RC\frac{dy(t)}{dt} + y(t)$  
from problem 2  
Homogenous : $y_h(t)= A \cdot e^{-\frac{t}{RC}}$  
Particular : $y_p(t)=\frac{1}{j\Omega RC+1}e^{j\Omega t}$  
General : $y(t)=[Ae^{-\frac{t}{RC}}+\frac{1}{j\omega RC+1}e^{j\omega t}]u(t)$  
Initial condition : t=0, u(0)=1  
$y(0)=0=A+\frac{1}{j\Omega RC+1}$  
$A=-\frac{1}{j\Omega RC+1}$(replace in y(t))  
$y(t)=-\frac{1}{j\Omega RC+1}(e^{-\frac{t}{RC}}-e^{j\Omega t})u(t)$  
$RC=1/800\pi \approx 2513$  
#### 1. 100Hz ($\Omega=2\pi f=200\pi$ )
from problem 3, $\alpha =0.25$  
$y(t)=\frac{1}{j0.25+1}[e^{j200\pi t}-e^{-800\pi t}]u(t)$   
$|H(200\pi)|\approx0.97$  
$\angle H(200\pi) = -\tan^{-1}(0.25)\approx -0.245 \ (rad)$  
$y(t)=0.97e^{-j0.245} (e^{j200\pi t}-e^{-2513t})u(t)$
#### 2. 400Hz ($\Omega=2\pi f=800\pi$ )
from problem 3  
$y(t)=\frac{1}{j+1}[e^{j800\pi t}-e^{-2513t}]u(t)$   
$|H(800\pi)|\approx 0.707$  
$\angle H(800\pi) \approx -0.785 \ (rad)$  
$y(t)=0.707e^{-j0.785} (e^{j800\pi t}-e^{-2513t})u(t)$
#### 3. 3000Hz ($\Omega=2\pi f=6000\pi$ )
from problem 3  
$y(t)=\frac{1}{j7.5+1}[e^{j6000\pi t}-e^{-800\pi t}]u(t)$   
$|H(200\pi)|\approx0.132$  
$\angle H(200\pi) = \approx -1.438 \ (rad)$  
$y(t)=0.132e^{-j1.438} (e^{j6000\pi t}-e^{-2513t})u(t)$

## Problem 5
### 題目
If $x[n] = e^{j\omega n}$ , please find the corresponding $y[n] = H(\omega)e^{j\omega n}$ for eq.(8) with sampling rates of 4000Hz, 8000Hz, and 16000Hz.($𝐻(𝜔)$ would be a discreate time Fourier transform! By handwriting mathematics. Please keep the calculation process on your paper).
### 手寫
![Screenshot_20251007_164521_Samsung Notes](https://hackmd.io/_uploads/HkR0jwfTgl.jpg)

### 解題
from Eq.8 $y[n]=\frac{RC}{RC+\tau}y[n-1]+\frac{\tau}{RC+\tau}x[n]$  
$H(\omega)e^{j\omega n}=\frac{RC}{RC+\tau}H(\omega)e^{j\omega (n-1)}+\frac{\tau}{RC+\tau}e^{j\omega n}$  (消$e^{j\omega n}$)  
$H(\omega)=\frac{RC}{RC+\tau}H(\omega)\cdot e^{-j\omega}+\frac{\tau}{RC+\tau}$  
$H(\omega)(1-\frac{RCe^{-j\omega}}{RC+\tau})=\frac{\tau}{RC+\tau}$  
$H(\omega)=\frac{\frac{\tau}{RC+\tau}}{1-\frac{RCe^{-j\omega}}{RC+\tau}}=\frac{\tau}{RC+\tau-RCe^{-j\omega}}$  
$y[t]=H(\omega)e^{j\omega n}=\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}$  
$RC=\frac{1}{800\pi}$  
$\tau:\text{sample period}$  
#### 4000 Hz , $\tau=\frac{1}{4000}$
$y[t]=\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}=\frac{\frac{1}{4000} e^{j\omega n} }{\frac{1}{800\pi}+\frac{1}{4000}-\frac{e^{-j\omega}}{800\pi}}\text{ , 同乘4000}\pi$  
$y[t]=\frac{\pi e^{j\omega n} }{5+\pi-5e^{-j\omega}}$  
#### 8000 Hz, $\tau=\frac{1}{8000}$
$y[t]=\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}=\frac{\frac{1}{8000} e^{j\omega n} }{\frac{1}{800\pi}+\frac{1}{8000}-\frac{e^{-j\omega}}{800\pi}}\text{ , 同乘8000}\pi$  
$y[t]=\frac{\pi e^{j\omega n} }{10+\pi-10e^{-j\omega}}$
#### 16000 Hz, $\tau=\frac{1}{16000}$
$y[t]=\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}=\frac{\frac{1}{16000} e^{j\omega n} }{\frac{1}{800\pi}+\frac{1}{16000}-\frac{e^{-j\omega}}{800\pi}}\text{ , 同乘16000}\pi$  
$y[t]=\frac{\pi e^{j\omega n} }{20+\pi-20e^{-j\omega}}$
## Problem 6
### 題目
If$x[n] = u[n] e^{j\omega n}$ , please find the corresponding $y[n] = H(\Omega) e^{j\omega n}$ for eq.(8) with sampling rates of 4000Hz, 8000Hz, and 16000Hz.(By handwriting mathematics. Please keep the calculation process on your paper).
### 手寫
![Screenshot_20251007_164530_Samsung Notes](https://hackmd.io/_uploads/HkDk3vGTee.jpg)

### 解題
from Eq.8,  $y[n]=\frac{RC}{RC+\tau}y[n-1]+\frac{\tau}{RC+\tau}x[n]$  
Homogenous $x[n]=0$  
$y[n]=\frac{RC}{RC+\tau}y[n-1]$  
$y[1]=\frac{RC}{RC+\tau}y[0]$  
$y[2]=\frac{RC}{RC+\tau}y[1]$  
$y[3]=\frac{RC}{RC+\tau}y[2]$  
......  
$y[n]=(\frac{RC}{RC+\tau})^n\cdot A$ \(A is constant)  
Particular  
from problem 5, $y_p[n]=H(\omega)e^{j\omega n}=\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}$  
General  
$y[n]=\{y_h[n]+y_p[n]\}u[n]=[A(\frac{RC}{RC+\tau})^n+\frac{\tau e^{j\omega n} }{RC+\tau-RCe^{-j\omega}}]u[n]$  
$RC=\frac{1}{800\pi}$  
$\tau=\text{sample period}$
#### 4000 Hz, $\tau=\frac{1}{4000}$  
$y[n]=[A(\frac{5}{5+\pi})^n+\frac{\pi e^{j\omega n} }{5+\pi-5e^{-j\omega}}]u[n]$  
#### 8000 Hz, $\tau=\frac{1}{8000}$  
$y[n]=[A(\frac{10}{10+\pi})^n+\frac{\pi e^{j\omega n} }{10+\pi-10e^{-j\omega}}]u[n]$  
#### 16000 Hz, $\tau=\frac{1}{16000}$  
$y[n]=[A(\frac{20}{20+\pi})^n+\frac{\pi e^{j\omega n} }{20+\pi-20e^{-j\omega}}]u[n]$  
## Problem 7
### 題目
Simulate the filtering of Problem 4 with eq.(8) by C programs. Please discuss the results made by the sampling rates of 4000Hz, 8000Hz, and 16000Hz.
### 程式
### sine_wav_gen.c
```
#include <stdio.h>
#include <string.h>
#include <math.h>
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
    fwrite("RIFF", sizeof(char), 4, fp);//0-3 
    fwrite(&chunkSize,sizeof(chunkSize),1,fp);//4-7 little endian
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
    if( !fp ) {// chech if the file is opened sucessfully
        fprintf(stderr, "Cannot save \n");
        exit(1);// stop and exit this program if error
    }
    for(n=0;n<N;n++){
        tmp=A*sin(2*PI*f*n*T); // x[t] 
        short left=(short)floor(tmp+0.5); //rounding
        fwrite( &left, sizeof(left), 1, fp);// write the waveform to the file

        tmp=A*cos(2*PI*f*n*T);
        short right=(short)floor(tmp+0.5); //rounding
        fwrite( &right, sizeof(right), 1, fp);// write the waveform to the file
    }

    fclose(fp);
}
```
#### include、define和file宣告
```
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#define PI 3.14159265359
FILE *fp;// a file point to save waveform
```
需要include以上來讓後面能用 sin/cos、檔案I/O等，並定義π和建立檔案指標。
#### 主程式
```
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
```
1. 首先檢查參數數量是否正確，否則錯誤並結束，接著將參數帶入fs、f等。
2. 印出目前檔案資訊、計算總取樣數N
3. 利用fopen()建立新的檔案與檢查是否正常。
4. 用add_header()來寫入wav header
5. 利用sine_wav()寫入PCM音檔資訊

#### add_header() 加上wav檔header
```
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
    fwrite("RIFF", sizeof(char), 4, fp);//0-3 
    fwrite(&chunkSize,sizeof(chunkSize),1,fp);//4-7 little endian
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
```
1. 收到取樣率和總取樣數後開始填入header到檔案fp裡
2. 區分header每個資訊需要幾byte 2byte用short 4byte用int
3. 將每項資訊填入或計算好
4. 利用fwrite()寫入 要注意端序與大小
#### sine_wav() 產生PCM音訊
```
void sine_wav(FILE *fp,int fs, int f, double L){
    //fs:sampling frequency f:frequency of sin wave L:length of sin wave in an unit of second
    double T = 1.0/fs;// sampling period 1/16k
    double A = 32767;// amplitude of sin wave
    double tmp;// temp variable
    int N = L*fs;// length of sin wave (sample)
    int n=0;//sample index
    // out[]:file name
    if( !fp ) {// chech if the file is opened sucessfully
        fprintf(stderr, "Cannot save \n");
        exit(1);// stop and exit this program if error
    }
    for(n=0;n<N;n++){
        tmp=A*sin(2*PI*f*n*T); // x[t] 
        short left=(short)floor(tmp+0.5); //rounding
        fwrite( &left, sizeof(left), 1, fp);// write the waveform to the file

        tmp=A*cos(2*PI*f*n*T);
        short right=(short)floor(tmp+0.5); //rounding
        fwrite( &right, sizeof(right), 1, fp);// write the waveform to the file
    }
```
1. 在收到取樣率、頻率、長度後初始化振幅大小為-32767到32767
2. 檢查檔案正常，否則退出
3. 輪流寫入左聲道sine波、右聲道cosine波 重複所有取樣數  
$x[t]=sin(2\cdot \pi \cdot f \cdot n T)$  
$x[t]=cos(2\cdot \pi \cdot f \cdot n T)$  

---
### RC_filtering.c
```
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
```
#### include、define
```
#include <stdio.h>
#include <stdlib.h>
#define PI 3.14159265359
```
include和定義PI

#### 檢查參數、建立檔案
```
 if (argc != 3) {
        fprintf(stderr, "Use: %s input.wav output.wav\n", argv[0]);
        return 1;
    }
    FILE *in_f, *out_f;
    in_f=fopen(argv[1],"rb");
    out_f=fopen(argv[2],"wb");
```
1. 檢查參數數量
2. 宣告輸入與輸出檔案
#### 讀取資訊
```
int sampleRate,subChunk2Size;
    short numChannels,bitsPerSample;
    fseek( in_f, 22, SEEK_SET );
    fread(&numChannels, 2, 1, in_f);
    fread(&sampleRate, 4, 1, in_f);
    fseek( in_f, 34, SEEK_SET );
    fread(&bitsPerSample, 2, 1, in_f);
    fseek( in_f, 40, SEEK_SET );
    fread(&subChunk2Size, 4, 1, in_f);
```
1. 利用header與fseek去尋找在header的資訊(sampleRate,subChunk2Size,numChannels,bitsPerSample)
#### 寫wav header
```
    fseek(in_f, 0, SEEK_SET);
    char header[44]; //same header
    fread(header, 1, 44, in_f);
    fwrite(header, 1, 44, out_f); 
```
找出輸入音檔的header，複製到輸出檔的header
#### 宣告變數
```
    double RC = 1.0/(800*PI);
    double tau = 1.0/sampleRate;
    double alpha = (RC/(RC+tau));
    double prevL= 0.0, prevR= 0.0;
    int numSamples=subChunk2Size*8/bitsPerSample;

    short *pcm=(short*)malloc(sizeof(short)*numSamples);
    short *fpcm=(short*)malloc(sizeof(short)*numSamples);// filterd PCM
```
宣告參數 $RC=\frac{1}{800\pi}$  
$\tau=\frac{1}{f_s}$  
$\alpha=\frac{RC}{RC+\tau}$  
PCM為輸入音檔、fpcm為輸出音檔PCM資料
#### 讀取音檔、warm up
```
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
```
1. 讀出PCM資訊
2. 先試7RC時常的sample幫RCfilter() warm up 可減少暫態
#### 透過RCfilter()通過濾波器
```
for(int i=0;i<numSamples;i+=2){
        fpcm[i]=RCfilter(pcm[i], &prevL,alpha);
        fpcm[i+1]=RCfilter(pcm[i+1], &prevR,alpha);
        printf("L:%7d R:%7d\n", fpcm[i], fpcm[i+1]);
    }

```
將實際PCM資訊傳進RCfilter()，左右聲道輪流寫進fpcm。
#### 寫入檔案、結尾
```
    fseek( out_f, 44, SEEK_SET );
    fwrite(fpcm, sizeof(short), numSamples, out_f);

    free(pcm); 
    free(fpcm);
    fclose(in_f); 
    fclose(out_f);
```
將fpcm寫進檔案，關閉檔案，釋放記憶體
#### RCfilter() 計算濾波
```
short RCfilter(short in, double *prev, double alpha){
    double out = alpha*(*prev) + (1-alpha)*in; // calculate output
    if (out > 32767.) out = 32767;
    else if (out < -32768) out = -32768;
    *prev = out;
    return (short)out;
}
```
透過公式 $y[n]=\frac{RC}{RC+\tau}y[n-1]+\frac{\tau}{RC+\tau}x[n]$ 算出濾波後的數字，如果超過上下限就限制大小。
## result
### 振幅比較
以下比較為改變fs(4k/8k/16k)和f(100/400/3000 Hz)的情況下，振幅與波形的差異° 濾波器已經先warm up 7RC，波形取中間段比較
#### fs=4000 
![4K1](https://hackmd.io/_uploads/HJvy7USple.png)
![4k4](https://hackmd.io/_uploads/rkdgm8Bplg.png)
![4k30](https://hackmd.io/_uploads/HkslQLSaeg.png)

在取樣率4000的情況下，100,400,3000Hz的比較
|  | 波形 | 振幅 |
| -------- | -------- | -------- |
| 100Hz     | 與原始波形相似| 振幅幾乎差不多  |
| 400Hz  |感覺有點delay 振幅也縮小了|振幅下降到大概7成|
|3000Hz |因為混疊，波形不太還原|下降到大約25%|


#### fs=8000 
![8k1](https://hackmd.io/_uploads/BkQ-XLSpxx.png)
![8k4](https://hackmd.io/_uploads/S1HWm8Bpge.png)
![8k30](https://hackmd.io/_uploads/S1uWQLrpel.png)

在取樣率8000的情況下,100,400,3000Hz的比較


| | 波形 | 振幅 |
| -------- | -------- | -------- |
| 100 Hz   | 波形十分接近    | 振幅幾乎相似   |
|400Hz|波形稍微delay，振幅下降約7成|振幅大約剩7成|
|3000Hz|取樣較少，波形較不規則|振幅大量衰弱|

#### fs=16000 
![16k1](https://hackmd.io/_uploads/SJCbXLBaex.png)
![16k4](https://hackmd.io/_uploads/Hkxfm8BTxg.png)
![16k30](https://hackmd.io/_uploads/Sk7zX8BTeg.png)


|  | 波形 | 振幅 |
| -------- | -------- | -------- |
| 100Hz   | 波形十分相似 | 振幅差不多  |
| 400Hz   | 波形稍微delay，振幅下降一些|振幅下降至7成 | 
| 3000Hz  | 波形振幅大量下降| 振幅衰弱至13%| 

*  振幅 out/in比較  
理論值 :  
f=100Hz (0.97)  
f=400Hz (0.707)  
f=3000Hz (0.132)  

| fs\f | 100Hz | 400Hz |3000Hz| 
| -------- | -------- | -------- | -------- | 
| 4000     | 0.9528    | 0.631 |x| 
| 8000| 0.9613| 0.6588| 0.1467| 
| 16000|0.9657 | 0.6812| 0.1302| 

* 可以得知，相同取樣率下，頻率越高，振幅衰減越大。 
* 相同頻率下，取樣率越高，越接近理論的振幅衰減，波形越完整。
* 討論-3db  
當訊號輸出功率下降至一半，即為-3db點。  
$20 \cdot \log_{10}{0.707}= -3$  
當功率下降至50%，為-3db，振幅下降比例為0.707  
可以得知在振幅下降到0.707時，功率已減少一半  
在problem 3中 f=400 振幅下降到0.707，因此400Hz為此濾波器的-3db點
### 相位比較
* 相位delay理論值  
f = 100 : -0.00039 (s)  
f = 400 : -0.000312(s)  
f = 3000 : -0.000076 (s)  

| fs\f | 100Hz | 400Hz |3000Hz| 
| -------- | -------- | -------- | -------- | 
| 4000     | -0.000365    | -0.000246 |0.000125| 
| 8000| -0.00037| -0.000279| -0.000018| 
| 16000|-0.000372 | -0.000294| -0.000046|

* 相同頻率下，fs 越高，模擬出的phase delay越接近理論值
* 相同取樣率下，f 越高，模擬出的phase delay會越小
## 總結
可以透過`./run.sh 16000 3000 0.01` (./run.sh fs f L) 執行sine_wav_gen.c後產生wav檔與txt檔紀錄samples，接著送進RC_filtering.c並產生濾波後的wav檔與txt檔紀錄samples，最後一起送進draw.py畫出波和比較。

在不同取樣率與頻率下比較振幅  
$|H|=\frac{out}{in}$  
| fs\f | 100Hz | 400Hz |3000Hz| 
| -------- | -------- | -------- | -------- | 
| 4000     | 0.9528    | 0.631 |x| 
| 8000| 0.9613| 0.6588| 0.1467| 
| 16000|0.9657 | 0.6812| 0.1302| 

 在不同取樣率與頻率下比較delay  
 $delay(s)=\frac{\angle H}{2\cdot \pi \cdot f}$
 | fs\f | 100Hz | 400Hz |3000Hz| 
| -------- | -------- | -------- | -------- | 
| 4000     | -0.000365    | -0.000246 |0.000125| 
| 8000| -0.00037| -0.000279| -0.000018| 
| 16000|-0.000372 | -0.000294| -0.000046|

  
* 相同取樣率下，頻率越高，振幅衰減越大。 
* 相同頻率下，取樣率越高，越接近理論的振幅衰減，波形越完整。
* 相同頻率下，fs 越高，模擬出的phase delay越接近理論值
* 相同取樣率下，f 越高，模擬出的phase delay會越小

本次透過比較不同頻率與取樣率情況下的波形、振幅、相位，透過先計算理論值後，在藉由程式模擬出訊號進入濾波器後的情況，最後畫出波來比較。可以藉此看到振幅衰減、相位延遲等現象。