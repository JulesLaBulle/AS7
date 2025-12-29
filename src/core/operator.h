#ifndef OPERATOR_H
#define OPERATOR_H

#include "oscillator.h"
#include "envelope.h"
#include "config.h"
#include "lut.h"

class Operator {
private:
    // Components
    Oscillator osc;
    Envelope env;
    
    // Configuration reference (modifiable in real-time)
    const OperatorConfig* config = nullptr;
    
    // Current base frequency (note frequency)
    float baseFrequency = 440.0f;
    float calculatedFrequency = 440.0f;

    // Velocity factor, calculated from velocity sensitivity and current velocity using LUT
    float velocityFactor = 1.0f;

    // Keyboard level scaling factor (converted from log domain)
    float levelScalingFactor = 1.0f;
    
    // Feedback state
    float feedbackLevel = 0.0f;      // Calculated from feedback parameter (0.0 - 1.0)
    float previousOutput = 0.0f;     // For feedback (delayed by one sample)

    // Amplitude modulation depth (from LFO)
    float ampModDepth = 0.0f;

    inline float midiToFrequency(uint8_t midiNote, float tuning = 440.0f) {
        // Convert MIDI note to frequency using A440 tuning
        return (tuning / 32.0f) * exp2f((static_cast<float>(midiNote) - 9.0f) / 12.0f);
    }
    
    // Recalculate frequency based on current configuration
    void updateFrequency() {
        if (!config) {
            calculatedFrequency = 0.0f;
            return;
        }
        
        const FrequencyConfig* freq = &config->frequency;
        
        // Calculate base ratio/frequency
        float baseFreq;
        float detuneMultiplier = 1.0f;
        if (freq->fixedFrequency) {
            // Fixed frequency mode
            baseFreq = FIXED_FREQ_BASE[freq->coarse % 4];
            
            // Apply fine adjustment (DX7 exact formula)
            // fine = 0-99, adds up to 10 times the base frequency (not linear !)
            baseFreq *= FIXED_FREQ_FINE_VALUES[freq->fine];
        } else {
            // Ratio mode
            float coarseValue;
            if (freq->coarse == 0) {
                coarseValue = 0.5f;  // DX7: 0 = 0.5 ratio
            } else {
                coarseValue = static_cast<float>(freq->coarse);
            }
            
            // Fine adds 0-99% of the coarse value
            float fineFactor = 1.0f + (static_cast<float>(freq->fine) / 100.0f);
            baseFreq = baseFrequency * coarseValue * fineFactor;

            // Apply detune using DX7 table (not linear!)
            // detune 7 = no detune, 0 = -detune, 14 = +detune
            if (freq->detune < 7) {
                // Negative detune: 1.0 - table[7 - detune]
                detuneMultiplier = 1.0f - DETUNE_TABLE[7 - freq->detune] / 1000.0f;
            } else if (freq->detune > 7) {
                // Positive detune: 1.0 + table[detune - 7]
                detuneMultiplier = 1.0f + DETUNE_TABLE[freq->detune - 7] / 1000.0f;
            }
        }
        
        calculatedFrequency = baseFreq * detuneMultiplier;
        
        // Set oscillator frequency
        osc.setFrequency(calculatedFrequency);
    }

    // RATE SCALING
    int scaleRate(uint8_t midinote, uint8_t sensitivity) {
        int x = std::min(31, std::max(0, static_cast<int>(midinote) / 3 - 7));  // From Dexed
        int qratedelta = (static_cast<int>(sensitivity) * x) >> 3;
        int rem = x & 7;
        if (sensitivity == 3 && rem == 3) {
            qratedelta -= 1;
        } else if (sensitivity == 7 && rem > 0 && rem < 4) {
            qratedelta += 1;
        }
        return qratedelta;
    }

    // VELOCITY SENSITIVITY
    float computeVelocityFactor(uint8_t velocity, uint8_t sensitivity) {
        velocity = std::min<uint8_t>(127, std::max<uint8_t>(1, velocity));
        sensitivity = std::min<uint8_t>(7, std::max<uint8_t>(0, sensitivity));

        if (velocity == VELOCITY_POINTS[0]) return VELOCITY_FACTOR_TABLE[sensitivity][0];
        if (velocity == VELOCITY_POINTS[8]) return VELOCITY_FACTOR_TABLE[sensitivity][8];

        int lowerIdx = 0, upperIdx = 0;
        for (int i = 0; i < 8; i++) {
            if (velocity <= VELOCITY_POINTS[i] && velocity > VELOCITY_POINTS[i + 1]) { // La table est a l'envers JE SAIS mais ca marche
                lowerIdx = i;
                upperIdx = i + 1;
                break;
            }
        }

        int velUpper = VELOCITY_POINTS[upperIdx];   
        int velLower = VELOCITY_POINTS[lowerIdx];

        float factorUpper = VELOCITY_FACTOR_TABLE[sensitivity][upperIdx];
        float factorLower = VELOCITY_FACTOR_TABLE[sensitivity][lowerIdx];

        float t = static_cast<float>(velocity - velUpper) / static_cast<float>(velLower - velUpper);
        return factorUpper + t * (factorLower - factorUpper);
    }

    // KEYBOARD LEVEL SCALING (from Dexed dx7note.cc)
    // Calculates level scaling factor based on MIDI note, breakpoint, depth, and curve
    // Returns a linear scaling factor (not exponential like original Dexed)
    // Called once per note trigger, so exp2f is acceptable
    // Dexed: outlevel = scaleoutlevel(outputLevel) + level_scaling, clamped to 127
    // scaleoutlevel(x) = x >= 20 ? 28 + x : lut[x]
    // For outputLevel 78: scaleoutlevel(78) = 106
    float scaleLevel(uint8_t midiNote, uint8_t outputLevel, uint8_t breakpoint, uint8_t leftDepth, uint8_t rightDepth,
                     uint8_t leftCurve, uint8_t rightCurve) {
        if (!leftDepth && !rightDepth) {
            return 1.0f;
        }

        int offset = static_cast<int>(midiNote) - static_cast<int>(breakpoint) - 17;
        
        int group, depth;
        uint8_t curve;
        
        if (offset >= 0) {
            // RIGHT side (more acute/higher notes)
            group = (offset + 1) / 3;  // Group by 3 semitones
            depth = rightDepth;
            curve = rightCurve;
        } else {
            // LEFT side (more grave/lower notes)
            group = -(offset - 1) / 3;  // Group by 3 semitones
            depth = leftDepth;
            curve = leftCurve;
        }
        
        // Limit group to available table size
        group = std::min(group, 99);
        
        // Calculate attenuation value based on curve type (matches Dexed ScaleCurve)
        int scale;
        if (curve == 0 || curve == 3) {
            // Linear curves
            scale = (group * depth * 329) >> 12;  // 329/4096 ≈ 0.080
        } else {
            // Exponential curves (1, 2)
            int raw_exp = KEYSCALE_EXP[std::min(group, 32)];
            scale = (raw_exp * depth * 329) >> 15;  // 329/32768 ≈ 0.010
        }
        
        if (curve < 2) {
            scale = -scale;
        }
        
        // Dexed: outlevel = scaleoutlevel(outputLevel), outlevel += scale, outlevel = min(127, outlevel)
        int scaledOutlevel = (outputLevel >= 20) ? (28 + outputLevel) : outputLevel;
        int clampedWithScale = std::min(127, scaledOutlevel + scale);
        int clampedNoScale = scaledOutlevel;
        
        // Factor = exp2((clamped_with_scale - clamped_no_scale) << 5 * INV_Q24)
        int effectiveScale = clampedWithScale - clampedNoScale;
        return LUT::exp2(static_cast<float>(effectiveScale << 5) * INV_Q24_ONE);
    }
    
public:
    // Default constructor
    Operator() = default;
    
    // -------------------------------------------------------------------------
    // Configuration Methods
    // -------------------------------------------------------------------------
    
    // Set operator configuration (can be called in real-time)
    void setConfig(const OperatorConfig* opConfig) {
        config = opConfig;
        if (config) {
            env.setConfig(&config->envelope);
        }
        updateFrequency();
    }
    
    // Dexed: fb_shift = 8 - feedback, scaled_fb = (y0 + y) >> (fb_shift + 1)
    void setFeedback(uint8_t feedbackValue) {
        if (feedbackValue > MAX_FEEDBACK_VALUE) {
            feedbackValue = MAX_FEEDBACK_VALUE;
        }
        feedbackLevel = FEEDBACK_TABLE[feedbackValue];
    }

    // Set amplitude modulation depth from LFO (called each sample by Voice)
    void setAmpMod(float ampMod) {
        ampModDepth = ampMod;
    }

    
    // -------------------------------------------------------------------------
    // Runtime Control Methods
    // -------------------------------------------------------------------------
    
    // Trigger the operator (start envelope attack)
    void trigger(uint8_t midiNote, uint8_t velocity) {
        baseFrequency = midiToFrequency(midiNote);
        updateFrequency();

        velocityFactor = computeVelocityFactor(velocity, config->velocitySensitivity);

        levelScalingFactor = scaleLevel(midiNote, config->envelope.outputLevel, config->lvlSclBreakpoint, 
                                        config->lvlSclLeftDepth, config->lvlSclRightDepth,
                                        config->lvlSclLeftCurve, config->lvlSclRightCurve);

        
        if(config->OSCKeySync) osc.reset();

        int qRateDelta = scaleRate(midiNote, config->envelope.rateScaling);
        env.setRateScaling(qRateDelta);
        env.trigger();  
        
        previousOutput = 0.0f;  // Reset feedback memory
    }
    
    // Release the operator (start envelope release)
    void release() {
        env.release();
    }
    
    // Reset operator to initial state
    void reset() {
        osc.reset();
        env.reset();
        previousOutput = 0.0f;
    }

    bool isActive() {
        return env.isActive();
    }
    
    // -------------------------------------------------------------------------
    // Audio Processing Methods
    // -------------------------------------------------------------------------
    
    // Process as carrier or modulator
    // phaseMod: accumulated phase modulation (can be any value, will be wrapped by oscillator)
    // pitchMod: frequency multiplier from LFO + pitch envelope (1.0 = no change)
    inline float process(float phaseMod = 0.0f, float pitchMod = 1.0f) {
        if (!config) return 0.0f;
        if (!config->on) return 0.0f;
        
        const float envelopeLevel = env.process();

        float oscillatorValue;
        
        if (phaseMod != 0.0f) {
            oscillatorValue = osc.process(phaseMod, pitchMod);
        } else {
            oscillatorValue = osc.process(0.0f, pitchMod);
        }

        float ampModFactor = ampModDepth * config->ampModSens * INV_PARAM_3;
        
        // Apply envelope, velocity scaling, and keyboard level scaling
        return oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor * (1.0f - ampModFactor) * OPERATOR_SCALING;
    }
    
    // Process with feedback (for feedback operator)
    // pitchMod: frequency multiplier from LFO + pitch envelope (1.0 = no change)
    inline float processWithFeedback(float pitchMod = 1.0f) {
        if (!config) return 0.0f;
        if (!config->on) return 0.0f;
        
        const float envelopeLevel = env.process();

        // Calculate phase modulation from feedback
        float phaseMod = 0.0f;
        if (feedbackLevel > 0.0f) {
            phaseMod = feedbackLevel * previousOutput * FEEDBACK_SCALING;
        }

        float oscillatorValue = osc.process(phaseMod, pitchMod);
        
        // Apply envelope, velocity, and level scaling to get the final output
        float gainedOutput = oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor;
        
        // Store output WITH gain/envelope for next sample's feedback
        previousOutput = gainedOutput * OPERATOR_SCALING;

        float ampModFactor = ampModDepth * config->ampModSens * INV_PARAM_3;

        return gainedOutput * (1.0f - ampModFactor) * OPERATOR_SCALING;
    }
};

#endif // OPERATOR_H