#ifndef LFO_H
#define LFO_H

#include "constants.h"
#include "config.h"
#include "lut.h"

// LFO with multiple waveforms for pitch and amplitude modulation
class LFO {
private:
    const LFOConfig* config = nullptr;
    float phase = 0.0f;
    float ampMod = 0.0f;
    float pitchMod = 1.0f;
    int delaySamples = 0;
    float sampleHoldValue = 0.0f;
    
    // Fast random (xorshift32)
    uint32_t randState = 12345;
    
    inline float fastRandom() {
        randState ^= randState << 13;
        randState ^= randState >> 17;
        randState ^= randState << 5;
        return static_cast<float>(randState) * 4.6566129e-10f * 2.0f - 1.0f;
    }

public:
    LFO() = default;

    void configure(const LFOConfig* lfoConfig) {
        config = lfoConfig;
    }

    void trigger() {
        phase = 0.0f;
        ampMod = 0.0f;
        pitchMod = 1.0f;
        if (config) {
            delaySamples = static_cast<int>(LFO_DELAY[config->delay] * SAMPLE_RATE);
        }
    }

    inline void process() {
        if (!config) return;

        if (delaySamples > 0) {
            --delaySamples;
            ampMod = 0.0f;
            pitchMod = 1.0f;
            return;
        }

        // Wrap phase
        if (phase >= 1.0f) phase -= 1.0f;
        
        float value;
        const uint8_t waveform = config->waveform;
        
        if (waveform == 0) {
            value = LUT::triangle(phase);
        } else if (waveform == 1) {
            value = -LUT::saw(phase);
        } else if (waveform == 2) {
            value = LUT::saw(phase);
        } else if (waveform == 3) {
            value = LUT::square(phase);
        } else if (waveform == 4) {
            value = LUT::sin(phase);
        } else {
            // Sample & Hold: update value on phase wrap
            phase += LFO_SPEED[config->speed] * INV_SAMPLE_RATE;
            if (phase >= 1.0f) {
                phase -= 1.0f;
                sampleHoldValue = fastRandom();
            }
            value = sampleHoldValue;
            
            ampMod = (value * 0.5f + 0.5f) * config->ampModDepth * INV_PARAM_99;
            pitchMod = LUT::exp2(value * config->pitchModDepth * INV_PARAM_99 * LFO_PMS[config->pitchModSens]);
            return;
        }

        ampMod = (value * 0.5f + 0.5f) * config->ampModDepth * INV_PARAM_99;
        pitchMod = LUT::exp2(value * config->pitchModDepth * INV_PARAM_99 * LFO_PMS[config->pitchModSens]);

        phase += LFO_SPEED[config->speed] * INV_SAMPLE_RATE;
    }

    inline float getAmpMod() const { return ampMod; }
    inline float getPitchMod() const { return pitchMod; }
};

#endif // LFO_H