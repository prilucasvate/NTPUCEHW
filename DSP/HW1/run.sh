#!/bin/bash
# usage: ./run.sh fs f L
# e.x. ./run.sh 8000 100 0.1

# check 
if [ $# -ne 3 ]; then
    echo "Usage: $0 <fs> <f> <L>"
    exit 1
fi

fs=$1
f=$2
L=$3

# file name
input="sincos_fs${fs}_f${f}_L${L}.wav"
output="filtered_sincos_fs${fs}_f${f}_L${L}.wav"

echo "======"
echo "fs = $fs"
echo "f  = $f"
echo "L  = $L"
echo "output: $output"
echo "========="
# compile
gcc -o sine_wav_gen.exe sine_wav_gen.c -lm
gcc -o RC_filtering.exe RC_filtering.c -lm
# run gen -> filter -> draw
./sine_wav_gen.exe "$fs" "$f" "$L" "$input" 1>wav_"$fs"_"$f"_"$L".txt\
&& ./RC_filtering.exe "$input" "$output" 1>filtered_"$fs"_"$f"_"$L".txt\
&& python3 draw.py "$input" "$output"