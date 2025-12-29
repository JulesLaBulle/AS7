#ifndef OPERATOR_H
#define OPERATOR_H

#include "oscillator.h"
#include "envelope.h"
#include "config.h"
#include "lut.h"
#include "lfo.h"

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

    // Keyboard level scaling
    float levelScalingFactor = 1.0f;
    
    // Feedback state
    float feedbackLevel = 0.0f;      // Calculated from feedback parameter (0.0 - 1.0)
    float previousOutput = 0.0f;     // For feedback (delayed by one sample)

    // LFO reference for modulation
    LFO* lfo = nullptr;

    inline float midiToFrequency(uint8_t midiNote, float tuning = 440.0f) {         // TODO: replace with LUT for speed ? full frequency table needed
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

    // KEYBOARD LEVEL SCALING
    float ScaleLevel(uint8_t midiNote, uint8_t breakpoint, uint8_t leftDepth, uint8_t rightDepth,
                     uint8_t leftCurve, uint8_t rightCurve) {
        int offset = midiNote - breakpoint;

        if (!offset || (!leftDepth && !rightDepth)) { return 1.0f; }

        const uint8_t depth  = offset < 0 ? leftDepth  : rightDepth;
        const uint8_t curve  = offset < 0 ? leftCurve  : rightCurve;

        const uint8_t tableVal = KEYSCALE_CURVES[curve][std::min(99, std::abs(offset))];

        const float attenuation = (tableVal * (depth / 99.0f));
        
        // TODO LUT and optimize ? maybe unnecessary, called only once per note played
        return exp2f(- attenuation * 0.75f / 6.0f);     
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
    
    // Set feedback level (0-7, DX7 style)
    void setFeedback(uint8_t feedbackValue) {
        if (feedbackValue > MAX_FEEDBACK_VALUE) {
            feedbackValue = MAX_FEEDBACK_VALUE;
        }
        feedbackLevel = (static_cast<float>(feedbackValue) / MAX_FEEDBACK_VALUE);
    }

    void setLFO(LFO* lfoPtr) {
        lfo = lfoPtr;
        osc.setLFO(lfoPtr);
    }

    
    // -------------------------------------------------------------------------
    // Runtime Control Methods
    // -------------------------------------------------------------------------
    
    // Trigger the operator (start envelope attack)
    void trigger(uint8_t midiNote, uint8_t velocity) {
        baseFrequency = midiToFrequency(midiNote);
        updateFrequency();

        velocityFactor = computeVelocityFactor(velocity, config->velocitySensitivity);

        levelScalingFactor = ScaleLevel(midiNote, config->lvlSclBreakpoint, config->lvlSclLeftDepth, config->lvlSclRightDepth,
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
    inline float process(float phaseMod = 0.0f) {
        if (!config) return 0.0f;
        if (!config->on) return 0.0f;
        
        const float envelopeLevel = env.process();
        std::cout << " envelope level: " << envelopeLevel << std::endl;

        float oscillatorValue;
        
        if (phaseMod != 0.0f) {
            oscillatorValue = osc.process(phaseMod);
        } else {
            oscillatorValue = osc.process();
        }

        float ampModFactor = lfo->getAmpMod() * config->ampModSens * INV_PARAM_3; // LFO amplitude modulation
        
        return oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor * (1.0f - ampModFactor) * OPERATOR_SCALING;
    }
    
    // Process with feedback (for feedback operator)
    inline float processWithFeedback() {
        if (!config) return 0.0f;
        if (!config->on) return 0.0f;
        
        const float envelopeLevel = env.process();
        std::cout << " envelope level: " << envelopeLevel << std::endl;

        float oscillatorValue;
        
        // Calculate phase modulation from feedback
        float phaseMod = 0.0f;
        if (feedbackLevel > 0.0f) {
            phaseMod = feedbackLevel * previousOutput * FEEDBACK_SCALING;
        }

        oscillatorValue = osc.process(phaseMod);
        previousOutput = oscillatorValue;  // Store output

        float ampModFactor = lfo->getAmpMod() * config->ampModSens * INV_PARAM_3; // LFO amplitude modulation

        return oscillatorValue * envelopeLevel * velocityFactor * levelScalingFactor * (1.0f - ampModFactor) * OPERATOR_SCALING;
    }
};

#endif // OPERATOR_H