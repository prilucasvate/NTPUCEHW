import numpy as np
import matplotlib.pyplot as plt

def plot_response():
    # 1. LPF parameters
    P = 1025
    wc = np.pi / 441  # cutoff frequency
    tau = (P - 1) / 2
    n = np.arange(P)

    # 2. FIR filter coefficients (Hamming window)
    x = n - tau
    with np.errstate(divide='ignore', invalid='ignore'):
        h_ideal = np.sin(wc * x) / (np.pi * x)
    h_ideal[int(tau)] = wc / np.pi 
    
    window = 0.54 - 0.46 * np.cos(2 * np.pi * n / (P - 1))
    h = h_ideal * window

    # 3. calculate frequency response
    # FFT points
    N_FFT = 65536 
    
    # FFT
    H = np.fft.fft(h, N_FFT)
    
    # shift zero frequency to center
    H_shifted = np.fft.fftshift(H)
    
    # caculate Magnitude (dB)
    # add a small value to avoid log(0)
    H_mag = 20 * np.log10(np.abs(H_shifted) + 1e-10) 
    
    # symmetric frequency axis
    w = np.linspace(-1, 1, N_FFT)

    # 4. plot
    plt.figure(figsize=(12, 6))
    plt.plot(w, H_mag, 'b', linewidth=1.5, label='FIR Filter')
    
    # Mark cutoff frequency
    cutoff_norm = (1/441) 
    plt.axvline(x=cutoff_norm, color='r', linestyle='--', label=f'Cutoff (+-{cutoff_norm:.4f})')
    plt.axvline(x=-cutoff_norm, color='r', linestyle='--')
    
    plt.title(r'Magnitude Response $|H(e^{j\omega})|$')
    plt.xlabel(r'Frequency ($\pi$)')
    plt.ylabel('Magnitude (dB)')
    plt.grid(True, which='both', linestyle='--', alpha=0.7)
    plt.legend()
    
    # only show -0.05 to 0.05 range
    plt.xlim(-0.05, 0.05) 
    plt.ylim(-100, 10) 
    
    plt.tight_layout()
    plt.savefig('magnitude_response.png')
    plt.show()
    print("magnitude_response.png saved.")

if __name__ == "__main__":
    plot_response()