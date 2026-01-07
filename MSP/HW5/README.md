# MMSP mini project-5 : Spectrogram practice
## Goal
1. Learn DFT、FFT、STFT
2. Generate spectrogram
3. Learn to use matplotlib

## Requirement
1. signal_gen.c (.exe)
    * s-8kHz.wav
    * s-16kHz.wav
2. spectrogram.c (.exe)
   * 16 ascii files (.txt)
   * {4 signals}.{Set1~Set4}.txt
3. spectshow.py
   * out_pdf files to show waveform and spectrogram
   * {4 signals}.{Set1~Set4}.pdf
4. Makefile
5. generate_all.yml
   * to generate all files automatically
6. README.md
## Git rule
1. Don't use -f 、 force
2. Conflict? wait for help
3. pull-> fix -> add -> commit -> push

## Division of work
### 林羽航
1. spectrogram.c
2. spectshow.py
3. fix Makefile
4. code review
5. write README.md
6. write result analysis
7. organize work group
8. to carry a heavy burden and move forward
9. teaching others how to use git
10. provide music for work group (Spotify playlist)
### 曾閎澤
1. provide chatGPT account
2. professional AI asker
3. responsible for saying "牛逼!!" when others finish their work
4. provide team food support (it's really important!!)
### 汪澤天
1. signal_gen.c
2. Makefile
3. generate_all.yml
4. provide work group environment
5. looking for late-night snacks
6. urge 曾閎澤 to finish the work on time
7. encourage team members

## Instructions
You can use `make` command to generate all required files automatically.  
signal_gen.c : generates the required 2 wave files.  
spectrogram.c : generates 16 ascii files (.txt) according to the 4 wave files and 4 parameter sets.  
spectshow.py : generates 16 pdf files (.pdf) to show waveform and spectrogram according to the 16 ascii files.  
Makefile : the makefile to run all the above programs automatically.  


## Results
All the required files are generated in the `result_files` folder.
### rectangular window vs hamming window
* 4 parameter sets:
  * Set1 & Set3 : rectangular window
  * Set2 & Set4 : hamming window
* Observations:
  * rectangular window : The spectral lines (frequency peaks) appear thinner and sharper, frequency resolution is higher. However, there are more side lobes (spectral leakage), the background noise level is higher.
  * hamming window : The spectral lines (frequency peaks) appear wider and smoother, frequency resolution is lower. However, side lobes (spectral leakage) are reduced, the background noise looks cleaner.
* Reasoning:
  * This phenomenon illustrates the trade-off between Main Lobe Width and Side Lobe Level (Spectral Leakage).
  * Rectangular Window: It has a narrow Main Lobe, providing better frequency resolution (thinner lines), which allows for precise identification of peak frequencies. However, it suffers from high Side Lobes, causing severe spectral leakage (visible background noise).
  * Hamming Window: By smoothing the signal edges, it suppresses the Side Lobes (reducing leakage). The trade-off is a wider Main Lobe, which results in the thicker frequency tracks, slightly reducing the frequency resolution.
* Conclusion:
  * rectangular window is better for frequency resolution, while hamming window is better for reducing spectral leakage and noise.
### sample rate: 8 kHz vs 16 kHz
* Observations:
  * 8 kHz : The spectrogram shows lower frequency content, with frequency components limited to a maximum of 4 kHz (Nyquist frequency). Besides, the time resolution appears coarser due to the lower sampling rate. Furthermore, sine wafe at 4000 Hz even disappear.
  * 16 kHz : The spectrogram displays higher frequency content, with frequency components extending up to 8 kHz. The time resolution is finer.
* Reasoning:
  * Nyquist Theorem : The sampling rate must be at least twice the highest frequency component in the signal to accurately capture it without aliasing. (Nyquist Frequency = $F_s / 2$)
  * Critical Sampling: When the sampling rate is exactly twice the signal frequency ($F_s = 2f$), sampling points may coincide with the zero-crossing points of a sine wave ($\sin(n\pi) = 0$), causing the signal to be completely missed.
* Conclusion:
  * 8 kHz sampling rate may miss higher frequency components and even critical frequencies, while 16 kHz sampling rate captures a higher frequency range.
### frame size: 32 ms vs 30 ms
* Observations:
  * 32 ms : The spectrogram shows better frequency resolution. The frequency components are more distinct. However, the time resolution is slightly reduced, making it harder to track rapid changes in the signal.
  * 30 ms : The spectrogram exhibits improved time resolution, allowing for better tracking of rapid changes in the signal. However, the frequency resolution is slightly reduced, leading to less distinct frequency components.
* Reasoning:
  * Time-Frequency Trade-off: Increasing the frame size improves frequency resolution (narrower frequency lines) but reduces time resolution (less precise timing of changes). Conversely, decreasing the frame size enhances time resolution (better tracking of rapid changes) but at the cost of frequency resolution (wider frequency lines).
* Conclusion:
  * A larger frame size is preferable for applications requiring better frequency resolution, while a smaller frame size is better suited for applications needing improved time resolution.

### computational complexity
* Calculation: 
  For a DFT size of $N$ and analysis window size of $P$:
  * Windowing : $P$ multiplications.
  * DFT Loop : The calculation iterates through $N/2 + 1$ frequency bins and $N$ time samples, requiring 2 multiplications (real & imaginary) per iteration.
  * Total : $(N/2 + 1) \times N \times 2 + P \approx N^2 + P$.