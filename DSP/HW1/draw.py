
# python3 dr2.py in.wav out.wav

import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import sys
import re
import warnings 

# ignore Warning
warnings.filterwarnings('ignore', message='This figure includes Axes that are not compatible with tight_layout')
def parse_filename(name):
    pattern = r"fs(\d+)_f([\d.]+)_L([\d.]+)" #e.x. sincos_fs4000_f100_L0.1.wav
    m = re.search(pattern, name)
    if not m:
        return None, None, None
    fs = int(m.group(1))
    f_str = m.group(2).rstrip(".")
    L_str = m.group(3).rstrip(".")
    f = float(f_str)
    L = float(L_str)
    return fs, f, L


def main(in_file, out_file):
    # --- find fs, f, L through filename---
    fs_n, f_n, L_n = parse_filename(in_file)
    if fs_n and f_n and L_n:
        print(f"fs={fs_n}, f={f_n}, L={L_n}")
    else:
        print("can't find fs/f/L")

    # --- read WAV file ---
    fs_in, data_in = wavfile.read(in_file)
    fs_out, data_out = wavfile.read(out_file)
    fs = fs_in

    # separate left, right data_in ->[N, 2]   [L, R]
    Lin  = data_in[:, 0].astype(np.float64) # L input
    Rin  = data_in[:, 1].astype(np.float64) # R input
    Lout = data_out[:, 0].astype(np.float64)
    Rout = data_out[:, 1].astype(np.float64)

    
    # --- phase response ( full signal) ---
    #  Lin , Lout  FFT (complex)
    fft_in  = np.fft.fft(Lin)
    fft_out = np.fft.fft(Lout)
    
    # ********  freqs *********
    N_fft = len(Lin)
    freqs = np.fft.fftfreq(N_fft, 1.0/fs) # [0Hz, fs/2Hz, fs, 3fs/2Hz, ...]
    if f_n is None:
        print("Error:frequency f is unknown.")
        return
    # find nearest frequency
    idx = np.argmin(np.abs(freqs - f_n))
    
    complex_in  = fft_in[idx] # find frequency complex value
    complex_out = fft_out[idx]
    
    H_f = complex_out / complex_in
    # amplitude
    ratio = np.abs(H_f)
    db = 20.0 * np.log10(ratio)
    
    phase_rad = np.angle(H_f)
    phase_delay = phase_rad / (2 * np.pi * f_n)

    # ---- output ----
    print("\n======== Amplitude Result  ========")
    print(f"Sample rate fs = {fs} Hz")
    print(f"Frequency f    = {f_n} Hz")
    print(f"|H| = A_out/A_in = {ratio:.6f}")
    print(f"20log10 |H|      = {db:.3f} dB")
    print("======== Phase Result  ========")
    print(f"Phase shift ∠H = {phase_rad:.6f} rad")
    print(f"Phase Delay  = {phase_delay:.6f} s")

    # --- draw ---
    t_in = np.arange(len(Lin)) / fs
    t_out = np.arange(len(Lout)) / fs

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharey=True, gridspec_kw={'hspace': 0.4})

    ax1.plot(t_in, Lin, label="Input Left (sin)", color='blue', alpha=0.7)
    ax1.plot(t_out, Lout, label="Output Left", color='orange', alpha=0.9)
    ax1.set_xlabel("Time [s]")
    ax1.set_ylabel("Amplitude")
    ax1.set_yticks(np.linspace(-32767, 32767, 9))
    ax1.grid(True, ls='--', alpha=0.5)
    ax1.legend(loc="upper right", frameon=True, framealpha=0.9)

    title_str = f"fs={fs}Hz, f={f_n}Hz, duration={L_n}s" if fs_n else f"fs={fs}Hz"
    fig.suptitle(title_str, fontsize=14, y=0.98)

    ax1.set_title("Left Channel", fontsize=13)

    ax2.plot(t_in, Rin, label="Input Right (cos)", color='green', alpha=0.7)
    ax2.plot(t_out, Rout, label="Output Right", color='red', alpha=0.9)
    ax2.set_xlabel("Time [s]")
    ax2.set_ylabel("Amplitude")
    ax2.grid(True, ls='--', alpha=0.5)
    ax2.legend(loc="upper right", frameon=True, framealpha=0.9)
    ax2.set_title("Right Channel", fontsize=13)

    plt.tight_layout(rect=[0, 0, 1, 0.95])
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: python3 dr2.py in.wav out.wav")
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])