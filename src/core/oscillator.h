#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "constants.h"
#include "lut.h"

// Phase accumulator oscillator with FM support
class Oscillator {
private:    
    float phase = 0.0f;
    float phaseInc = 0.0f;  // Cached: frequency * INV_SAMPLE_RATE
    
public:
    void setFrequency(float freq) {
        // Clamp frequency to valid range and precompute phase increment
        if (freq < 0.0f) freq = 0.0f;
        else if (freq > 20000.0f) freq = 20000.0f;
        phaseInc = freq * INV_SAMPLE_RATE;
    }
    
    float getFrequency() const { return phaseInc * SAMPLE_RATE; }
    
    void reset() { phase = 0.0f; }
    
    // Process with phase modulation and pitch multiplier
    // pitchMod: frequency multiplier (1.0 = no change, 2.0 = octave up)
    inline float process(float phaseMod, float pitchMod) {
        // Modulated phase for output
        float modulatedPhase = phase + phaseMod;
        
        // Single conditional wrap (handles ±1 range from typical FM modulation)
        if (modulatedPhase >= 1.0f) modulatedPhase -= 1.0f;
        else if (modulatedPhase < 0.0f) modulatedPhase += 1.0f;

        // Advance base phase with pitch modulation
        phase += phaseInc * pitchMod;
        
        // Wrap base phase
        if (phase >= 1.0f) phase -= 1.0f;
        else if (phase < 0.0f) phase += 1.0f;
        
        return LUT::sin(modulatedPhase);
    }
};

#endif // OSCILLATOR_H