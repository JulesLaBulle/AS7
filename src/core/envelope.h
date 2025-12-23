#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "constants.h"
#include "lut.h"

class Envelope {
private:
    // Internal constants
    static constexpr uint8_t levelLUT[20] = {0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46};

    static constexpr int statics[84] = { 1764000, 1764000, 1411200, 1411200, 1190700, 1014300, 992250,
                            882000, 705600, 705600, 584325, 507150, 502740, 441000, 418950,
                            352800, 308700, 286650, 253575, 220500, 220500, 176400, 145530,
                            145530, 125685, 110250, 110250, 88200, 88200, 74970, 61740,
                            61740, 55125, 48510, 44100, 37485, 31311, 30870, 27562, 27562,
                            22050, 18522, 17640, 15435, 14112, 13230, 11025, 9261, 9261, 7717,
                            6615, 6615, 5512, 5512, 4410, 3969, 3969, 3439, 2866, 2690, 2249,
                            1984, 1896, 1808, 1411, 1367, 1234, 1146, 926, 837, 837, 705,
                            573, 573, 529, 441, 441
                            // and so on, values from Dexed for accurate DX7 envelopes when targetLevel = currentLevel
    };

    // Reference to configuration (modifiable in real-time, need to call update() after changes)
    const EnvelopeConfig* config = nullptr;
    
    // Process made for 44.1kHz sample rate, need to re-scale if different
    static constexpr uint32_t SR_multiplier = static_cast<uint32_t>(44100.0f / SAMPLE_RATE * Q24_ONE);

    // Levels and rates
    uint8_t levels[4] = {0};
    uint8_t rates[4] = {0};
    int outputLevel = 0;

    uint32_t currentLevel = 0;
    int increment = 0;
    int targetLevel = 0;

    uint8_t currentState = 4;  // 0: attack, 1: decay1, 2: decay2, 3: release, 4: idle
    bool rising = false;

    bool keyDown = false;
    int rateScaling = 0;

    int staticCount = 0;

    bool initialised = false;

    // Go to specified state and calculate increment
    void goToState(uint8_t newState) {
        currentState = newState;

        if(currentState < 4) {
            uint8_t newLevel = levels[currentState];
            int actualLevel = static_cast<int>(scaleOutLevel(newLevel)) >> 1;
            actualLevel = (actualLevel << 6) + outputLevel - 4256;    // Magic (from Dexed)
            actualLevel = actualLevel < 16 ? 16 : actualLevel;

            targetLevel = actualLevel << 16;
            rising = (static_cast<uint32_t>(targetLevel) > currentLevel);

            // Rate calculation with scaling
            int qRate = (static_cast<int>(rates[currentState]) * 41) >> 6;
            qRate += rateScaling;
            qRate = std::min(qRate, 63);

            // Accurate DX7 envelope when targetLevel = currentLevel (from Dexed)
            if (static_cast<uint32_t>(targetLevel) == currentLevel || (currentState == 0 && newLevel == 0)) {
                // approximate number of samples at 44.100 kHz to achieve the time
                // empirically gathered using 2 TF1s, could probably use some double-checking
                // and cleanup, but it's pretty close for now.
                int staticRate = static_cast<int>(rates[currentState]);
                staticRate += rateScaling; // needs to be checked, as well, but seems correct
                staticRate = std::min(staticRate, 99);
                staticCount = staticRate < 77 ? statics[staticRate] : 20 * (99 - staticRate);
                if (staticRate < 77 && (currentState == 0 && newLevel == 0)) {
                    staticCount /= 20; // attack is scaled faster
                }
                staticCount = static_cast<int>((static_cast<int64_t>(staticCount) * static_cast<int64_t>(SR_multiplier)) >> 24);    //64bit mult can be too slow? 
            }
            else {
                staticCount = 0;
            }

            increment = (4 + (qRate & 3)) << (2 + /*LG_N +*/ (qRate >> 2));     // Using LG_N for 64 samples buffer process. Dexed works with 64 samples buffer, but AS7 doesn't
            increment = static_cast<int>((static_cast<int64_t>(increment) * static_cast<int64_t>(SR_multiplier)) >> 24);
        }
    }

    // Scale output level from 0-99 to internal level representation (non-linear)
    uint8_t scaleOutLevel(uint8_t outlevel) {
        return outlevel >= 20 ? 28 + outlevel : levelLUT[outlevel];
    } 

public:
    Envelope() = default;

    // Set envelope configuration
    void setConfig(const EnvelopeConfig* envConfig) {
        initialised = true;
        config = envConfig;

        levels[0] = config->l1;
        levels[1] = config->l2;
        levels[2] = config->l3;
        levels[3] = config->l4;
        rates[0] = config->r1;
        rates[1] = config->r2;
        rates[2] = config->r3;
        rates[3] = config->r4;
        outputLevel = (scaleOutLevel(config->outputLevel) << 5);
        currentLevel = 0;
        staticCount = 0;
        goToState(4);
    }

    // Update envelope parameters (call after changing config)
    void update(int rateScalingInput = 0) {     // TODO: maybe optimize if called often ? LazyUpdate ?
        if (!config) return;

        levels[0] = config->l1;
        levels[1] = config->l2;
        levels[2] = config->l3;
        levels[3] = config->l4;
        rates[0] = config->r1;
        rates[1] = config->r2;
        rates[2] = config->r3;
        rates[3] = config->r4;
        outputLevel = (scaleOutLevel(config->outputLevel) << 5);
        rateScaling = rateScalingInput;
        
        // If currently in a state, recalculate increment
        goToState(currentState);
    }
    
    // Set rate scaling (based on midinote, called from Operator)
    void setRateScaling(int rateScalingInput) {
        rateScaling = rateScalingInput;
        
        // If currently in a state, recalculate increment
        goToState(currentState);
    }

    // Trigger the envelope (start attack phase)
    void trigger() {
        keyDown = true;
        goToState(0);
    }
    
    // Release the envelope (start release phase)
    void release() {
        keyDown = false;
        if (currentState < 3) { goToState(3); }
    }
    
    // Process one sample, return current envelope level
    inline float process() {
        if (!config || !initialised) return 0.0f;
        
        // Pause when targetLevel == currentLevel (from Dexed)
        if (staticCount) {
            staticCount -= 1;  // Replace by N to process N-sized buffer at once
            if (staticCount <= 0) {
                staticCount = 0;
                goToState(currentState + 1);
            }
        }

        if (currentState < 3 || ((currentState < 4) && !keyDown)) {
            if (staticCount > 0) {
                ;   // Wait until staticCount reaches 0
            }
            else if (rising) {
                const int jumptarget = 1716;
                if (currentLevel < (jumptarget << 16)) {    // Jump to value if below
                    currentLevel = jumptarget << 16;
                }
                currentLevel += (((17 << 24) - currentLevel) >> 24) * increment;
                if (currentLevel >= static_cast<uint32_t>(targetLevel)) {
                    currentLevel = targetLevel;
                    goToState(currentState + 1);
                }
            }
            else {  // !rising
                currentLevel -= increment;
                if (currentLevel <= static_cast<uint32_t>(targetLevel)) {
                    currentLevel = targetLevel;
                    goToState(currentState + 1);
                }
            }
        }

        return LUT::exp2(static_cast<float>(currentLevel) * INV_Q24_ONE - 14.0f);   // Convert Q24 to float and scale down
    }
    
    // Reset envelope to idle state
    void reset() {
        goToState(4);
        currentLevel = 0;
    }  
    
    // Get current envelope state
    uint8_t getState() const { return currentState; }

    // An envelope is active if it's been initialised and either it hasn't reached L4 yet or L4 > 0
    bool isActive() { return initialised && (currentState < 4 || (currentState == 4 && levels[3] > 0)); }
};

#ifndef ENVELOPE_STATIC_DEFINED
#define ENVELOPE_STATIC_DEFINED
constexpr uint8_t Envelope::levelLUT[20];
constexpr int Envelope::statics[84];
#endif

#endif // ENVELOPE_H