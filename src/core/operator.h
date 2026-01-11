#ifndef OPERATOR_H
#define OPERATOR_H

#include "oscillator.h"
#include "envelope.h"
#include "config.h"
#include "lut.h"

// FM operator: oscillator + envelope with velocity/level scaling
class Operator {
private:
    Oscillator osc;
    Envelope env;
    
    const OperatorConfig* config = nullptr;
    
    // Cached values (computed on trigger, not per-sample)
    float calculatedFrequency = 440.0f;
    float velocityFactor = 1.0f;
    float levelScalingFactor = 1.0f;
    float feedbackLevel = 0.0f;
    float previousOutput = 0.0f;
    
    // Cached config values for hot path
    float cachedAmpModSens = 0.0f;
    bool isOn = false;

    static inline float midiToFrequency(uint8_t midiNote) {
        return 13.75f * exp2f((static_cast<float>(midiNote) - 9.0f) / 12.0f);
    }
    
    void updateFrequency(float baseFrequency) {
        if (!config) {
            calculatedFrequency = 0.0f;
            return;
        }
        
        const FrequencyConfig* freq = &config->frequency;
        float baseFreq;
        float detuneMultiplier = 1.0f;
        
        if (freq->fixedFrequency) {
            baseFreq = FIXED_FREQ_BASE[freq->coarse % 4] * FIXED_FREQ_FINE_VALUES[freq->fine];
        } else {
            const float coarseValue = (freq->coarse == 0) ? 0.5f : static_cast<float>(freq->coarse);
            const float fineFactor = 1.0f + static_cast<float>(freq->fine) * 0.01f;
            baseFreq = baseFrequency * coarseValue * fineFactor;

            if (freq->detune != 7) {
                const int detuneIdx = (freq->detune < 7) ? (7 - freq->detune) : (freq->detune - 7);
                const float detuneAmount = DETUNE_TABLE[detuneIdx] * 0.001f;
                detuneMultiplier = (freq->detune < 7) ? (1.0f - detuneAmount) : (1.0f + detuneAmount);
            }
        }
        
        calculatedFrequency = baseFreq * detuneMultiplier;
        osc.setFrequency(calculatedFrequency);
    }

    static int scaleRate(uint8_t midinote, uint8_t sensitivity) {
        int x = static_cast<int>(midinote) / 3 - 7;
        if (x < 0) x = 0;
        else if (x > 31) x = 31;
        
        int qratedelta = (static_cast<int>(sensitivity) * x) >> 3;
        const int rem = x & 7;
        if (sensitivity == 3 && rem == 3) --qratedelta;
        else if (sensitivity == 7 && rem > 0 && rem < 4) ++qratedelta;
        return qratedelta;
    }

    float computeVelocityFactor(uint8_t velocity, uint8_t sensitivity) const {
        if (velocity < 1) velocity = 1;
        else if (velocity > 127) velocity = 127;
        if (sensitivity > 7) sensitivity = 7;

        if (velocity == VELOCITY_POINTS[0]) return VELOCITY_FACTOR_TABLE[sensitivity][0];
        if (velocity == VELOCITY_POINTS[8]) return VELOCITY_FACTOR_TABLE[sensitivity][8];

        for (int i = 0; i < 8; ++i) {
            if (velocity <= VELOCITY_POINTS[i] && velocity > VELOCITY_POINTS[i + 1]) {
                const float t = static_cast<float>(velocity - VELOCITY_POINTS[i + 1]) / 
                               static_cast<float>(VELOCITY_POINTS[i] - VELOCITY_POINTS[i + 1]);
                return VELOCITY_FACTOR_TABLE[sensitivity][i + 1] + 
                       t * (VELOCITY_FACTOR_TABLE[sensitivity][i] - VELOCITY_FACTOR_TABLE[sensitivity][i + 1]);
            }
        }
        return VELOCITY_FACTOR_TABLE[sensitivity][0];
    }

    float scaleLevel(uint8_t midiNote, uint8_t outputLevel, uint8_t breakpoint, 
                     uint8_t leftDepth, uint8_t rightDepth,
                     uint8_t leftCurve, uint8_t rightCurve) const {
        if (!leftDepth && !rightDepth) return 1.0f;

        const int offset = static_cast<int>(midiNote) - static_cast<int>(breakpoint) - 17;
        
        int group, depth;
        uint8_t curve;
        
        if (offset >= 0) {
            group = (offset + 1) / 3;
            depth = rightDepth;
            curve = rightCurve;
        } else {
            group = -(offset - 1) / 3;
            depth = leftDepth;
            curve = leftCurve;
        }
        
        if (group > 99) group = 99;
        
        int scale;
        if (curve == 0 || curve == 3) {
            scale = (group * depth * 329) >> 12;
        } else {
            const int raw_exp = KEYSCALE_EXP[(group > 32) ? 32 : group];
            scale = (raw_exp * depth * 329) >> 15;
        }
        
        if (curve < 2) scale = -scale;
        
        const int scaledOutlevel = (outputLevel >= 20) ? (28 + outputLevel) : outputLevel;
        int clampedWithScale = scaledOutlevel + scale;
        if (clampedWithScale > 127) clampedWithScale = 127;
        
        const int effectiveScale = clampedWithScale - scaledOutlevel;
        return LUT::exp2(static_cast<float>(effectiveScale << 5) * INV_Q24_ONE);
    }
    
public:
    Operator() = default;
    
    void setConfig(const OperatorConfig* opConfig) {
        config = opConfig;
        if (config) {
            env.setConfig(&config->envelope);
            isOn = config->on;
            cachedAmpModSens = config->ampModSens * INV_PARAM_3;
        }
    }
    
    void setFeedback(uint8_t feedbackValue) {
        if (feedbackValue > MAX_FEEDBACK_VALUE) feedbackValue = MAX_FEEDBACK_VALUE;
        feedbackLevel = FEEDBACK_TABLE[feedbackValue];
    }
    
    void trigger(uint8_t midiNote, uint8_t velocity) {
        const float baseFrequency = midiToFrequency(midiNote);
        updateFrequency(baseFrequency);

        velocityFactor = computeVelocityFactor(velocity, config->velocitySensitivity);
        levelScalingFactor = scaleLevel(midiNote, config->envelope.outputLevel, config->lvlSclBreakpoint, 
                                        config->lvlSclLeftDepth, config->lvlSclRightDepth,
                                        config->lvlSclLeftCurve, config->lvlSclRightCurve);
        
        if (config->OSCKeySync) osc.reset();

        env.setRateScaling(scaleRate(midiNote, config->envelope.rateScaling));
        env.trigger();
        previousOutput = 0.0f;
    }
    
    void release() { env.release(); }
    
    void reset() {
        osc.reset();
        env.reset();
        previousOutput = 0.0f;
    }

    bool isActive() const { return env.isActive(); }
    
    // Process with modulation - ampMod passed directly to avoid per-sample setter
    inline float process(float phaseMod, float pitchMod, float ampMod) {
        if (!isOn) return 0.0f;
        
        const float envelopeLevel = env.process();
        const float oscillatorValue = osc.process(phaseMod, pitchMod);
        const float ampModFactor = ampMod * cachedAmpModSens;
        
        return oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor * (1.0f - ampModFactor);
    }
    
    // Process with feedback - ampMod passed directly
    inline float processWithFeedback(float pitchMod, float ampMod) {
        if (!isOn) return 0.0f;
        
        const float envelopeLevel = env.process();
        
        const float phaseMod = feedbackLevel * previousOutput * FEEDBACK_SCALING;
        const float oscillatorValue = osc.process(phaseMod, pitchMod);
        const float gainedOutput = oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor;
        
        previousOutput = gainedOutput;
        const float ampModFactor = ampMod * cachedAmpModSens;

        return gainedOutput * (1.0f - ampModFactor);
    }
};

#endif // OPERATOR_H