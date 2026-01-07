#!/bin/bash

# --- variables ---
SRC_REC="fftsrc.c"
SRC_ITER="fftsrc2.c"
EXE_REC="fft_rec"
EXE_ITER="fft_iter"
WAV_FILE="blue_giant_fragment_44.4kHz_16bits_stereo.wav"

# --- check files ---
if [ ! -f "$WAV_FILE" ]; then
    echo "can't find '$WAV_FILE'"
fi

echo "========================================"
echo "compile..."
echo "========================================"

# 1. Compile recursive version (old)
# -O2: enable optimization to see the algorithmic difference
gcc "$SRC_REC" -o "$EXE_REC" -lm -O2
if [ $? -eq 0 ]; then
    echo "[Recursive] $SRC_REC compiled successfully -> ./$EXE_REC"
else
    echo "[Recursive] compilation failed!"
    exit 1
fi

# 2. Compile iterative version
gcc "$SRC_ITER" -o "$EXE_ITER" -lm -O2
if [ $? -eq 0 ]; then
    echo "[Iterative] $SRC_ITER compiled successfully -> ./$EXE_ITER"
else
    echo "[Iterative] compilation failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "Start performance test"
echo "========================================"

# Run (recursive)
echo "Running recursive version (Recursive FFT)..."
./"$EXE_REC"
echo "----------------------------------------"

# Run (iterative)
echo "Running iterative version (Iterative FFT)..."
./"$EXE_ITER"

echo "========================================"
echo "Test finished!"
echo "Output files: output.wav and output2.wav "