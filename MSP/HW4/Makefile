
#  Makefile for Encoder and Decoder

CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm

# --- Executable Names ---
ENCODER = encoder
DECODER = decoder

# --- Source Files (include logger.c) ---
ENC_SRC = encoder.c logger.c
DEC_SRC = decoder.c logger.c

# --- File Names ---
IN_FILE = input.txt
OUT_FILE = decoded.txt
CODEBOOK = codebook.csv
ENCODED_FILE = encoded.bin


#  Rules

all: test

# diff after generating OUT_FILE
test: $(OUT_FILE)
	@echo "--- [ 比較 $(IN_FILE) 和 $(OUT_FILE) ] ---"
	@# diff -s 檔案相同時輸出 "Files are identical"
	diff -s $(IN_FILE) $(OUT_FILE) 
	@echo "--- [ 測試完成 ] ---"

# generate output file (decoded.txt)
# depends on Encoded file and Decoder executable
$(OUT_FILE): $(ENCODED_FILE) $(DECODER)
	@echo "--- [ 執行 Decoder... ] ---"
	@# Decoder 參數順序: <Output> <Codebook> <Input>
	./$(DECODER) $(OUT_FILE) $(CODEBOOK) $(ENCODED_FILE) > decoder.log 2>&1

# generate Encoded file
# depends on Codebook
$(ENCODED_FILE): $(CODEBOOK)

# generate Codebook
$(CODEBOOK): $(ENCODER) $(IN_FILE)
	@echo "--- [ 執行 Encoder... ] ---"
	@# Encoder 參數順序: <Input> <Codebook> <Output>
	./$(ENCODER) $(IN_FILE) $(CODEBOOK) $(ENCODED_FILE) > encoder.log 2>&1

# compile Encoder
$(ENCODER): $(ENC_SRC) logger.h
	@echo "--- [ 編譯 Encoder... ] ---"
	$(CC) $(CFLAGS) -o $(ENCODER) $(ENC_SRC) $(LDFLAGS)

# compile Decoder
$(DECODER): $(DEC_SRC) logger.h
	@echo "--- [ 編譯 Decoder... ] ---"
	$(CC) $(CFLAGS) -o $(DECODER) $(DEC_SRC) $(LDFLAGS)

# clean up generated files
clean:
	@echo "--- [ 清除檔案... ] ---"
	rm -f $(ENCODER) $(DECODER) $(OUT_FILE) $(CODEBOOK) $(ENCODED_FILE)

.PHONY: all test clean