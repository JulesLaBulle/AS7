#!/bin/bash

# Test multiple presets to ensure stability across different FM configurations

cd "$(dirname "$0")" || exit 1

echo "=== Testing Multiple DX7 Presets ==="
echo ""

# Presets to test: 0=BRASS 1, 5=CLARINET, 10=E.PIANO 1, 15=STRINGS 1, 20=BELL, 25=VIBES, 31=CLAV
PRESETS=(0 5 10 15 20 25 31)

for PRESET in "${PRESETS[@]}"; do
    echo "Testing Preset #$PRESET..."
    
    # Create temporary C++ file with this preset
    sed "s/constexpr uint8_t PRESET_NUMBER = [0-9]*;/constexpr uint8_t PRESET_NUMBER = $PRESET;/" \
        src/pc/main_pc.cpp > src/pc/main_pc_test.cpp
    
    # Quick compile (already has object files)
    g++ -O3 -std=c++17 \
        -I. \
        src/pc/main_pc_test.cpp \
        build/sine_test.o \
        -o build/fm_synth_test 2>&1 | grep -i error && {
        echo "  ❌ Compilation failed for preset #$PRESET"
        continue
    }
    
    # Run test
    timeout 30s ./build/fm_synth_test > /tmp/test_$PRESET.log 2>&1
    RESULT=$?
    
    if [ $RESULT -eq 124 ]; then
        echo "  ❌ Timeout (infinite loop?)"
    elif [ $RESULT -ne 0 ]; then
        echo "  ❌ Crashed (exit code: $RESULT)"
        tail -20 /tmp/test_$PRESET.log
    else
        # Check for NaN or Inf in output
        if tail -5 /tmp/test_$PRESET.log | grep -qE "(nan|inf|NaN|Inf)"; then
            echo "  ⚠️  Contains NaN/Inf values"
            tail -5 /tmp/test_$PRESET.log
        else
            SAMPLES=$(tail -3 /tmp/test_$PRESET.log | grep "Samples" | awk '{print $3}')
            echo "  ✅ Success ($SAMPLES samples)"
        fi
    fi
done

echo ""
echo "=== Test Complete ==="
rm -f src/pc/main_pc_test.cpp
