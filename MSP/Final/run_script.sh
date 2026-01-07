#!/bin/bash

# exit when any command fails
set -e

# Define file name variables for easy management
INPUT_IMG="Kimberly.bmp"
GOLDEN_Q_IMG="QResKimberly.bmp" # Standard quantized image produced by Mode 1(a), used for comparison with Mode 2/3

echo "========================================"
echo " 0. Preparing Environment..."
echo "========================================"

# Check if input image exists, if not download it (suitable for GitHub Actions environment)
if [ ! -f "$INPUT_IMG" ]; then
    echo "[Info] $INPUT_IMG not found. Downloading..."
    curl -o $INPUT_IMG https://raw.githubusercontent.com/cychiang-ntpu/mmsp-2025-final-jpeg/main/Kimberly.bmp
fi

# clean and build
make clean
make
echo "[Pass] Compilation successful."

echo ""
echo "========================================"
echo " 1. Testing Mode 0 (Basic I/O)..."
echo "========================================"
# Mode 0: Basic I/O Test
echo "--- Starting Mode 0 encoder ---"
./encoder 0 $INPUT_IMG R.txt G.txt B.txt dim.txt
echo ""
echo "--- Starting Mode 0 decoder ---"
./decoder 0 ResKimberly_M0.bmp R.txt G.txt B.txt dim.txt

# check if the output image matches the input image
if diff -q $INPUT_IMG ResKimberly_M0.bmp > /dev/null; then
    echo "[Pass] Mode 0: Images are identical."
else
    echo "[Fail] Mode 0: Images differ!"
    exit 1
fi

echo ""
echo "========================================"
echo " 2. Testing Mode 1 (DCT/Quantization)..."
echo "========================================"
# Encoder 1
echo "--- Starting Mode 1 encoder ---"
./encoder 1 $INPUT_IMG Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw

# Decoder 1(a) - Lossy (standard quantization QResKimberly.bmp)
echo ""
echo "--- Starting Mode 1(a) decoder ---"
./decoder 1 $GOLDEN_Q_IMG $INPUT_IMG Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw

# Decoder 1(b) - Lossless (Error Compensation)
echo ""
echo "--- Starting Mode 1(b) decoder ---"
./decoder 1 ResKimberly_M1b.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw

# check if the output image from Mode 1(b) matches the input image
if diff -q $INPUT_IMG ResKimberly_M1b.bmp > /dev/null; then
    echo "[Pass] Mode 1(b): Lossless reconstruction images matches Kimberly.bmp"
else
    echo "[Fail] Mode 1(b): Images differ!"
    exit 1
fi

echo ""
echo "========================================"
echo " 3. Testing Mode 2 (DPCM/RLE)..."
echo "========================================"

# --- Mode 2(a) ASCII ---
echo "--- Starting Mode 2(a) encoder ---"
./encoder 2 $INPUT_IMG ascii rle_code.txt
echo ""
echo "--- Starting Mode 2(a) decoder ---"
./decoder 2 QResKimberly_M2_asc.bmp ascii rle_code.txt

# Compare: Mode 2 decoded result must be equal to Mode 1(a) result
if diff -q $GOLDEN_Q_IMG QResKimberly_M2_asc.bmp > /dev/null; then
    echo "[Pass] Mode 2 ASCII matches QResKimberly.bmp"
else
    echo "[Fail] Mode 2 ASCII output differs from QResKimberly.bmp"
    exit 1
fi

# --- Mode 2(b) Binary ---
echo ""
echo "--- Starting Mode 2(b) encoder ---"
./encoder 2 $INPUT_IMG binary rle_code.bin
echo ""
echo "--- Starting Mode 2(b) decoder ---"
./decoder 2 QResKimberly_M2_bin.bmp binary rle_code.bin

# compare: Mode 2 decoded result must be equal to Mode 1(a) result
if diff -q $GOLDEN_Q_IMG QResKimberly_M2_bin.bmp > /dev/null; then
    echo "[Pass] Mode 2 Binary matches QResKimberly.bmp"
else
    echo "[Fail] Mode 2 Binary output differs from QResKimberly.bmp"
    exit 1
fi

echo ""
echo "========================================"
echo " 4. Testing Mode 3 (Huffman)..."
echo "========================================"
# we use prefix "code_asc" and "code_bin" to differentiate output files

# --- Mode 3(a) ASCII ---
# Prefix named code_asc -> will generate code_asc_Y_DC.txt ...
echo "--- Starting Mode 3(a) encoder ---"
echo "[Note] code_asc is codebook prefix for ASCII mode."
./encoder 3 $INPUT_IMG ascii code_asc huffman_out.txt
echo ""
echo "--- Starting Mode 3(a) decoder ---"
./decoder 3 QResKimberly_M3_asc.bmp ascii code_asc huffman_out.txt

if diff -q $GOLDEN_Q_IMG QResKimberly_M3_asc.bmp > /dev/null; then
    echo "[Pass] Mode 3 ASCII matches QResKimberly.bmp"
else
    echo "[Fail] Mode 3 ASCII output differs from QResKimberly.bmp"
    exit 1
fi

# --- Mode 3(b) Binary ---
echo ""
echo "--- Starting Mode 3(b) encoder ---"
echo "[Note] code_bin is codebook prefix for Binary mode."
./encoder 3 $INPUT_IMG binary code_bin huffman_out.bin
echo ""
echo "--- Starting Mode 3(b) decoder ---"
./decoder 3 QResKimberly_M3_bin.bmp binary code_bin huffman_out.bin

if diff -q $GOLDEN_Q_IMG QResKimberly_M3_bin.bmp > /dev/null; then
    echo "[Pass] Mode 3 Binary matches QResKimberly.bmp"
else
    echo "[Fail] Mode 3 Binary output differs from QResKimberly.bmp"
    exit 1
fi

echo ""
echo "========================================"
echo " All Tests Passed Successfully! "
echo "========================================"