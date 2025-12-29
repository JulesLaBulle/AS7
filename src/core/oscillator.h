#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "constants.h"
#include "lut.h"

class Oscillator {
private:    
    float phase = 0.0f;
    float frequency = 0.0f;
    
public:
    void setFrequency(float freq) {
        frequency = freq;
        if (frequency < 0.0f) frequency = 0.0f;
        if (frequency > 20000.0f) frequency = 20000.0f;
    }
    
    float getFrequency() const { return frequency; }
    
    void reset() { phase = 0.0f; }
    
    // Process with optional phase modulation and pitch multiplier
    // pitchMod: multiplier for frequency (1.0 = no change, 2.0 = octave up, 0.5 = octave down)
    inline float process(float phaseMod = 0.0f, float pitchMod = 1.0f) {
        // Calculate modulated phase: base phase + phase modulation
        float modulatedPhase = phase + phaseMod;
        
        // Wrap modulated phase to [0, 1] range
        while (modulatedPhase >= 1.0f) modulatedPhase -= 1.0f;
        while (modulatedPhase < 0.0f) modulatedPhase += 1.0f;

        // Increment base phase for next sample (with pitch modulation)
        phase += frequency * pitchMod * INV_SAMPLE_RATE;
        
        // Wrap phase to [0, 1] for next sample
        if(phase < 0.0f) phase += 1.0f;
        if(phase >= 1.0f) phase -= 1.0f;
        
        return LUT::sin(modulatedPhase);
    }
};

#endif // OSCILLATOR_H