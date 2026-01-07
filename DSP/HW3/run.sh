#!/bin/bash

echo "=============================================="
echo "[Step 1] Compiling C Programs..."
echo "=============================================="

if [ -f "f_src.c" ]; then
    echo "Compiling Optimize Version (f_src.c)..."
    gcc -o f_src.exe f_src.c -lm
else
    echo "Error: f_src.c not found!"
fi

if [ -f "ff_src.c" ]; then
    echo "Compiling Fastest Version (ff_src.c)..."
    gcc -o ff_src.exe ff_src.c -lm
else
    echo "Error: ff_src.c not found!"
fi


if [ -f "src.c" ]; then
    echo "Compiling Original Version..."
    gcc -o src.exe src.c -lm
else
    echo "Warning: src.c not found, skipping."
fi

echo ""
echo "=============================================="
echo "[Step 2] Run & compare speed (fast) ..."
echo "=============================================="

if [ -f "./f_src.exe" ]; then
    echo ""
    echo "--- Running Optimize Version ---"
    ./f_src.exe
fi

if [ -f "./ff_src.exe" ]; then
    echo ""
    echo "--- Running Fastest Version ---"
    ./ff_src.exe
fi

echo ""
echo "=============================================="
echo "[Step 3] Plotting ..."
echo "=============================================="

if command -v python3 &> /dev/null; then
    echo "1. Drawing Magnitude Response..."
    python3 LPF_response.py
    
    echo "2. Drawing Waveform ..."
    python3 waveform.py
else
    echo "Error: python3 not found."
fi

echo ""

echo "=============================================="
echo "[Step 4] Run & compare speed (slow) ..."
echo "=============================================="

if [ -f "./src.exe" ]; then
    echo "--- Running Original Version ---"
    echo "it may took a long time, you can use shorter waveform for testing or stop it anytime (Ctrl + C)."
    ./src.exe
fi
echo ""
echo "=============================================="
echo "All Done! Check .png files."
echo "=============================================="
