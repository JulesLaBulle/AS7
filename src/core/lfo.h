#ifndef LFO_H
#define LFO_H

#include "constants.h"
#include "config.h"
#include "lut.h"

class LFO {
private:
    const LFOConfig* config = nullptr;
    float phase = 0.0f;
    float ampMod = 0.0f;
    float pitchMod = 0.0f;
    int delaySamples = 0;

public:
    LFO() = default;

    void configure(const LFOConfig* lfoConfig) {
        config = lfoConfig;
    }

    void trigger() {
        phase = 0.0f;
        ampMod = 0.0f;
        pitchMod = 0.0f;
        delaySamples = static_cast<int>(LFO_DELAY[config->delay] * SAMPLE_RATE); // Delay in samples
    }

    inline void process() {
        if(!config) {
            return;
        }

        if(delaySamples > 0) {
            --delaySamples;
            ampMod = 0.0f;
            pitchMod = 1.0f;
            return;
        }

        float value = 0.0f;

        switch(config->waveform) {
            case 0: // Triangle
                if(phase >= 1.0f) phase -= 1.0f;
                value = LUT::triangle(phase);
                break;
            case 1: // Saw Down
                if(phase >= 1.0f) phase -= 1.0f;
                value = LUT::saw(phase) * -1.0f;
                break;
            case 2: // Saw Up
                if(phase >= 1.0f) phase -= 1.0f;
                value = LUT::saw(phase);
                break;
            case 3: // Square
                if(phase >= 1.0f) phase -= 1.0f;
                value = LUT::square(phase);
                break;
            case 4: // Sine
                if(phase >= 1.0f) phase -= 1.0f;
                value = LUT::sin(phase);
                break;
            case 5: // Sample & Hold
                if(phase >= 1.0f) {
                    phase -= 1.0f;
                    value = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f; // Random value between -1 and 1
                    // TODO: optimize with better random generator
                }
                break;
        }

        // TODO: test and add scaling factors if needed
        ampMod = (value * 0.5f + 0.5f) * config->ampModDepth * INV_PARAM_99;                        // Normalized to 0-1 and scaled on AMD parameter
        pitchMod = LUT::exp2(value * config->pitchModDepth * INV_PARAM_99 * LFO_PMS[config->pitchModSens]);    // Scaled on PMD parameter

        phase += LFO_SPEED[config->speed] * INV_SAMPLE_RATE;
    }

    inline float getAmpMod() { return ampMod; }

    inline float getPitchMod() { return pitchMod; }
};

#endif // LFO_H