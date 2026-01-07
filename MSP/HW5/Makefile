CC = gcc
CFLAGS = -Wall -O2
PY = python3
CURL = curl -L --fail --silent --show-error
# --- 設定輸出資料夾名稱 ---
OUT_DIR = result_files

# --- 下載連結 ---
AEUEO_8K_URL  = https://raw.githubusercontent.com/cychiang-ntpu/mini-project-5-spectrogram/main/aeueo-8Hz.wav
AEUEO_16K_URL = https://raw.githubusercontent.com/cychiang-ntpu/mini-project-5-spectrogram/main/aeueo-16kHz.wav

AEUEO_8K_WAV  = aeueo-8kHz.wav
AEUEO_16K_WAV = aeueo-16kHz.wav
S_8K_WAV      = s-8kHz.wav
S_16K_WAV     = s-16kHz.wav

WAVS = $(AEUEO_8K_WAV) $(AEUEO_16K_WAV) $(S_8K_WAV) $(S_16K_WAV)

# --- 主要目標 ---
default: run

all_progs: spectrogram signal_gen

spectrogram: spectrogram.c
	$(CC) $(CFLAGS) -o spectrogram spectrogram.c -lm

signal_gen: signal_gen.c
	$(CC) $(CFLAGS) -o signal_gen signal_gen.c -lm

download: $(AEUEO_8K_WAV) $(AEUEO_16K_WAV)

$(AEUEO_8K_WAV):
	@echo "== Download $(AEUEO_8K_WAV) =="
	$(CURL) -o $(AEUEO_8K_WAV) "$(AEUEO_8K_URL)"
	@python3 -c "import sys; d=open('$(AEUEO_8K_WAV)','rb').read(12); print(d); sys.exit(0 if d[:4]==b'RIFF' and d[8:12]==b'WAVE' else 1)" \
	  || (echo "ERROR: $(AEUEO_8K_WAV) is not a RIFF/WAVE file"; exit 1)

$(AEUEO_16K_WAV):
	@echo "== Download $(AEUEO_16K_WAV) =="
	$(CURL) -o $(AEUEO_16K_WAV) "$(AEUEO_16K_URL)"
	@python3 -c "import sys; d=open('$(AEUEO_16K_WAV)','rb').read(12); print(d); sys.exit(0 if d[:4]==b'RIFF' and d[8:12]==b'WAVE' else 1)" \
	  || (echo "ERROR: $(AEUEO_16K_WAV) is not a RIFF/WAVE file"; exit 1)

gen: all_progs
	@echo "== Generate s-8kHz.wav and s-16kHz.wav =="
	./signal_gen

# ---  run：建立資料夾 ---
run: download gen
	@echo "== Creating output directory: $(OUT_DIR) =="
	@mkdir -p $(OUT_DIR)
	@echo "== Generate 16 spectrogram TXT + PDF into $(OUT_DIR)/ =="
	@set -e; \
	for wav in $(WAVS); do \
		base="$${wav%.wav}"; \
		echo "Processing $$wav ..."; \
		\
		./spectrogram 32 rectangular 32 10 "$$wav" "$(OUT_DIR)/$$base.Set1.txt"; \
		$(PY) spectshow.py "$$wav" "$(OUT_DIR)/$$base.Set1.txt" "$(OUT_DIR)/$$base.Set1.pdf"; \
		\
		./spectrogram 32 hamming     32 10 "$$wav" "$(OUT_DIR)/$$base.Set2.txt"; \
		$(PY) spectshow.py "$$wav" "$(OUT_DIR)/$$base.Set2.txt" "$(OUT_DIR)/$$base.Set2.pdf"; \
		\
		./spectrogram 30 rectangular 32 10 "$$wav" "$(OUT_DIR)/$$base.Set3.txt"; \
		$(PY) spectshow.py "$$wav" "$(OUT_DIR)/$$base.Set3.txt" "$(OUT_DIR)/$$base.Set3.pdf"; \
		\
		./spectrogram 30 hamming     32 10 "$$wav" "$(OUT_DIR)/$$base.Set4.txt"; \
		$(PY) spectshow.py "$$wav" "$(OUT_DIR)/$$base.Set4.txt" "$(OUT_DIR)/$$base.Set4.pdf"; \
	done; \
	echo "== All Done! Check the '$(OUT_DIR)' folder. =="

# --- clean：清除資料夾 ---
clean:
	rm -f spectrogram signal_gen
	rm -rf $(OUT_DIR)

distclean: clean
	rm -f $(S_8K_WAV) $(S_16K_WAV)

.PHONY: default all_progs download gen run clean distclean