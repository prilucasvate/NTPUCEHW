import sys
import wave
import numpy as np
import matplotlib.pyplot as plt

DYNAMIC_RANGE = 80      # show range in dB

if len(sys.argv) != 4:
    print("Usage: python3 tt.py <wav> <txt> <pdf>")
    sys.exit(1)

in_wav, in_txt, out_pdf = sys.argv[1], sys.argv[2], sys.argv[3]

# 1. read WAV
with wave.open(in_wav, 'r') as f:
    frames = f.readframes(f.getnframes())
    signal = np.frombuffer(frames, dtype=np.int16)
    time_axis = np.linspace(0, f.getnframes() / f.getframerate(), num=f.getnframes())
    framerate = f.getframerate()

# 2. text to spectrogram data
spec_data = np.loadtxt(in_txt).T  # read and transpose

# 3. plotting settings
plt.style.use('dark_background')
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
plt.subplots_adjust(hspace=0.3) # slightly increase spacing to avoid title crowding

# --- upper plot: waveform ---
ax1.plot(time_axis, signal, color='darkgreen', linewidth=0.8)
ax1.set_xlim(time_axis[0], time_axis[-1])

# set title and Y axis label
ax1.set_title('Time Domain (Waveform)', color='white', fontsize=12)
ax1.set_ylabel('Amplitude', color='white') # <--- add Amplitude

#  upper plot border (only keep left for Amplitude)
ax1.spines['top'].set_visible(False)
ax1.spines['right'].set_visible(False)
ax1.spines['bottom'].set_visible(False)
ax1.spines['left'].set_color('white')
ax1.tick_params(axis='y', colors='white')

# --- lower plot: spectrogram ---
vmax = np.max(spec_data)
vmin = vmax - DYNAMIC_RANGE

ax2.imshow(spec_data, aspect='auto', origin='lower', cmap='gray',
           vmin=vmin, vmax=vmax,
           extent=[time_axis[0], time_axis[-1], 0, framerate/2])

# set title and labels
ax2.set_title('Frequency Domain (Spectrogram)', color='white', fontsize=12) # <--- (Spectrogram)
ax2.set_xlabel('Time (s)', color='white')
ax2.set_ylabel('Frequency (Hz)', color='white')

# lower plot border
ax2.tick_params(colors='white')
ax2.spines['top'].set_visible(False)
ax2.spines['right'].set_visible(False)
ax2.spines['bottom'].set_color('white')
ax2.spines['left'].set_color('white')

# save to PDF
plt.savefig(out_pdf, facecolor='black', bbox_inches='tight')
print(f"Done: {out_pdf}")