import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

def plot_waveforms():
    # 1. read input.wav and output.wav
    try:
        rate_in, data_in = wavfile.read('blue_giant_fragment_44.4kHz_16bits_stereo.wav')
        rate_out, data_out = wavfile.read('output.wav')
    except FileNotFoundError:
        print("can't find input.wav or output.wav. ")
        return

    N_view = 100 # plot first 100 samples
    
    
    # 2. handle Input Data, only left channel if stereo
    if len(data_in.shape) > 1: 
        y_in = data_in[:N_view, 0]
    else:
        y_in = data_in[:N_view]
        
    # build Input time axis
    time_in = np.arange(len(y_in)) / rate_in


    # Output Data
    # output points = input points * rate_out / rate_in
    N_out_view = int(N_view * rate_out / rate_in)
    
    if len(data_out.shape) > 1:
        y_out = data_out[:N_out_view, 0]
    else:
        y_out = data_out[:N_out_view]

    # output time axis
    time_out = np.arange(len(y_out)) / rate_out
    

    # 3. plot
    plt.figure(figsize=(10, 8))

    # Input 
    plt.subplot(2, 1, 1)
    plt.plot(time_in, y_in, 'go-', label='Input Points') # green + line + circle
    plt.title(f'Input Waveform (First {N_view} samples {rate_in} Hz)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend()

    # Output 
    plt.subplot(2, 1, 2)
    plt.plot(time_out, y_out, 'bo-', label='Output Points') # blue + line + circle
    plt.title(f'Output Waveform ({rate_out} Hz)')
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.savefig('waveform.png')  # save figure
    plt.show()
    print("waveform.png saved.")

if __name__ == "__main__":
    plot_waveforms()