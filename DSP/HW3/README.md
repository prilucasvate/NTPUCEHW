# DSPHW3 Sample Rate Conversion

## Goal
* upsample and downsample input audio
* implement FIR filter 
* check the response and waveform
* change input audio (44.1kHz, 16bits) to output audio (8kHz, 16bits)

## Files
* `src.c`: main C code for sample rate conversion 
* `f_src.c`: main C code for sample rate conversion faster version
* `ff_src.c`: main C code for sample rate conversion fastest version
* `run.sh`: bash script to compile and run the code
* `blue_giant_fragment_44.4kHz_16bits_stereo.wav`: input audio file (44.1kHz, 16bits)
* `input.wav`: testing input sinewave (44.1kHz, 16bits)
* `output.wav`: output audio file (8kHz, 16bits)
* `LPF_response.py`: plot the filter response
* `waveform.py`: plot the waveform before and after sample rate conversion

## Introduction
You can use `run.sh` to compile and run the code. The output audio file will be generated as `output.wav`. In addition, two python scripts are provided to plot the filter response `magnitude_response.png` and waveform `waveform.png` . 

`run.sh` will run the faster version `f_src.c` first, and then plot the waveform and filter response. After that, it will run the original version `src.c`. Note that the original version may take a long time to finish. You can use a shorter waveform for testing or stop it anytime .

`sine_waveform.png` is the waveform of the testing sine wave `input.wav`. It show better than `waveform.png`, because the music waveform don't have any sound at the beginning. So it's a straight line at the beginning.

## Results
By checking the `waveform.png`, we can see that the output waveform is similar to the input waveform, and the sample rate conversion is correctly. Around 8 samples in input waveform correspond to 1 sample in output waveform, which is correct since we downsampled the audio from 44.1kHz to 8kHz.
### Filter Design
The low-pass filter is designed using the windowed-sinc method. The filter length is set to 1025 taps. The cutoff frequency is set to $\frac{\pi}{441}$ , which corresponds to 4kHz in the original 44.1kHz sampling rate. A Hamming window is applied to the sinc function to reduce side-lobe levels and improve the filter's performance.
The filter response is shown in `magnitude_response.png`. The cutoff frequency is $\frac{\pi}{441}$. 
Due to the nyquist theorem, the maximum frequency after downsampling to 8kHz is 4kHz.  
The filter can filter out the high frequency components above 4kHz, which can avoid aliasing during downsampling. 

### Optimization Explanation
Three versions of code are provided: original version `src.c`, faster version `f_src.c `, and fastest version `ff_src.c`.  
The original version and faster version produce the same output audio `output.wav`. The faster version by caculating near output samples only, so it is much faster than the original version. The fastest version further optimize the code by removing unnecessary boundary checks, making it even more efficient. It only process the necessary input samples for each output sample directly.

The fastest version idea is : Filter index = (m * M) - (k * L) + shift  
where shift = (P-1)/2 to center the filter.  
(m * M) - (k * L) is the distance between the output sample and input sample in terms of the upsampled index.  
For each output sample m, we can calculate the k range that will produce non-zero contributions to this output sample.
We set the base = m * M + (P - 1) / 2, then we can derive the k range as follows:  
k_max : $base - k \times L \geq 0$ => $k \leq \frac{base}{L}$  
k_min : $base - k \times L \leq P-1$ => $k \geq \frac{base - (P-1)}{L}$

So we can directly calculate k_min and k_max for each output sample m, and only iterate k from k_min to k_max. And the speed is from original version $O(N_{in} \times N_{out})$ to fastest version $O(N_{out} \times \frac{P}{L})$ , where P is the filter length, L is the downsampling factor.