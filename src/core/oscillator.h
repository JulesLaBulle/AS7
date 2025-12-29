#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "constants.h"
#include "lut.h"
#include "lfo.h"

class Oscillator {
private:    
    float phase = 0.0f;
    float frequency = 0.0f;
    LFO* lfo = nullptr;
    
public:
    void setFrequency(float freq) {
        frequency = freq;
        if (frequency < 0.0f) frequency = 0.0f;
        if (frequency > 20000.0f) frequency = 20000.0f;
    }
    
    float getFrequency() const { return frequency; }
    
    void reset() { phase = 0.0f; }

    void setLFO(LFO* lfoPtr) {
        lfo = lfoPtr;
    }
    
    inline float process(float phaseMod = 0.0f) {
        // Calculate modulated phase: base phase + phase modulation
        // Both are in [0, 1] range
        float modulatedPhase = phase + phaseMod;
        
        // Wrap modulated phase to [0, 1] range to prevent phase accumulation artifacts
        // This is critical to prevent delirious values when multiple operators modulate
        while (modulatedPhase >= 1.0f) modulatedPhase -= 1.0f;
        while (modulatedPhase < 0.0f) modulatedPhase += 1.0f;

        // Increment base phase for next sample (with LFO pitch modulation)
        phase += frequency * lfo->getPitchMod() * INV_SAMPLE_RATE;
        
        // Wrap phase to [0, 1] for next sample
        if(phase < 0.0f) phase += 1.0f;
        if(phase >= 1.0f) phase -= 1.0f;
        
        return LUT::sin(modulatedPhase);
    }
};

#endif // OSCILLATOR_H