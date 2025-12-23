#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# List all source files
SOURCES="
src/pc/main_pc.cpp
"

# Maximum optimization flags for execution speed
CXXFLAGS="-std=c++11 -O3 -march=native -ffast-math -flto"
CXXFLAGS+=" -fno-exceptions -fno-rtti -fomit-frame-pointer"
CXXFLAGS+=" -funroll-loops -ftree-vectorize"

# Warning flags for code quality
CXXFLAGS+=" -Wall -Wextra -Wpedantic -Wshadow"
CXXFLAGS+=" -Wconversion -Wnull-dereference -Wlogical-op"
CXXFLAGS+=" -Wduplicated-cond -Wduplicated-branches"

# For ARM microcontrollers (Teensy), uncomment:
# CXXFLAGS+=" -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16"
# CXXFLAGS+=" -D__ARM_ARCH"

# Compile with g++
g++ $CXXFLAGS -o build/fm_synth $SOURCES -I./src -lm

# Check compilation result
if [ $? -eq 0 ]; then
    echo "Compilation successful"
    echo "Executable size: $(wc -c < build/fm_synth) bytes"
    echo "Launching program..."
    
    # Run the program
    ./build/fm_synth
else
    echo "Compilation failed"
    exit 1
fi