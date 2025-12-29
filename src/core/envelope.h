#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "constants.h"
#include "lut.h"

// DX7-style ADSR envelope with accurate timing from Dexed
class Envelope {
private:
    static constexpr uint8_t levelLUT[20] = {0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46};

    static constexpr int statics[77] = {
        1764000, 1764000, 1411200, 1411200, 1190700, 1014300, 992250,
        882000, 705600, 705600, 584325, 507150, 502740, 441000, 418950,
        352800, 308700, 286650, 253575, 220500, 220500, 176400, 145530,
        145530, 125685, 110250, 110250, 88200, 88200, 74970, 61740,
        61740, 55125, 48510, 44100, 37485, 31311, 30870, 27562, 27562,
        22050, 18522, 17640, 15435, 14112, 13230, 11025, 9261, 9261, 7717,
        6615, 6615, 5512, 5512, 4410, 3969, 3969, 3439, 2866, 2690, 2249,
        1984, 1896, 1808, 1411, 1367, 1234, 1146, 926, 837, 837, 705,
        573, 573, 529, 441, 441
    };

    // Reference to configuration
    const EnvelopeConfig* config = nullptr;
    
    // Sample rate scaling (Q24 format)
    static constexpr uint32_t SR_multiplier = static_cast<uint32_t>(44100.0f / SAMPLE_RATE * Q24_ONE);

    // Cached config values for hot path
    uint8_t levels[4] = {0};
    uint8_t rates[4] = {0};
    int outputLevel = 0;

    // Runtime state
    uint32_t currentLevel = 0;
    int increment = 0;
    int targetLevel = 0;
    int staticCount = 0;
    int rateScaling = 0;
    uint8_t currentState = 4;  // 0-3: ADSR, 4: idle
    bool rising = false;
    bool keyDown = false;
    bool initialised = false;

    void goToState(uint8_t newState) {
        currentState = newState;
        if (currentState >= 4) return;

        const uint8_t newLevel = levels[currentState];
        int actualLevel = static_cast<int>(scaleOutLevel(newLevel)) >> 1;
        actualLevel = (actualLevel << 6) + outputLevel - 4256;
        if (actualLevel < 16) actualLevel = 16;

        targetLevel = actualLevel << 16;
        rising = (static_cast<uint32_t>(targetLevel) > currentLevel);

        // Rate calculation with scaling
        int qRate = (static_cast<int>(rates[currentState]) * 41) >> 6;
        qRate += rateScaling;
        if (qRate > 63) qRate = 63;

        // Static timing for equal levels (from Dexed)
        if (static_cast<uint32_t>(targetLevel) == currentLevel || (currentState == 0 && newLevel == 0)) {
            int staticRate = static_cast<int>(rates[currentState]) + rateScaling;
            if (staticRate > 99) staticRate = 99;
            
            staticCount = (staticRate < 77) ? statics[staticRate] : 20 * (99 - staticRate);
            if (staticRate < 77 && currentState == 0 && newLevel == 0) {
                staticCount /= 20;
            }
            staticCount = static_cast<int>((static_cast<int64_t>(staticCount) * static_cast<int64_t>(SR_multiplier)) >> 24);
        } else {
            staticCount = 0;
        }

        increment = (4 + (qRate & 3)) << (2 + (qRate >> 2));
        increment = static_cast<int>((static_cast<int64_t>(increment) * static_cast<int64_t>(SR_multiplier)) >> 24);
    }

    uint8_t scaleOutLevel(uint8_t outlevel) const {
        return (outlevel >= 20) ? (28 + outlevel) : levelLUT[outlevel];
    }

public:
    Envelope() = default;

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
        outputLevel = scaleOutLevel(config->outputLevel) << 5;
        currentLevel = 0;
        staticCount = 0;
        goToState(4);
    }

    void update(int rateScalingInput = 0) {
        if (!config) return;
        levels[0] = config->l1;
        levels[1] = config->l2;
        levels[2] = config->l3;
        levels[3] = config->l4;
        rates[0] = config->r1;
        rates[1] = config->r2;
        rates[2] = config->r3;
        rates[3] = config->r4;
        outputLevel = scaleOutLevel(config->outputLevel) << 5;
        rateScaling = rateScalingInput;
        goToState(currentState);
    }
    
    void setRateScaling(int rateScalingInput) {
        rateScaling = rateScalingInput;
        goToState(currentState);
    }

    void trigger() {
        keyDown = true;
        goToState(0);
    }
    
    void release() {
        keyDown = false;
        if (currentState < 3) goToState(3);
    }
    
    // Process one sample - optimized hot path
    inline float process() {
        if (!initialised) return 0.0f;
        
        // Handle static timing (equal level pause)
        if (staticCount > 0) {
            --staticCount;
            if (staticCount == 0) goToState(currentState + 1);
            return LUT::exp2(static_cast<float>(currentLevel) * INV_Q24_ONE - 14.0f);
        }

        // Process envelope stages
        const bool shouldProcess = (currentState < 3) || (currentState == 3 && !keyDown);
        
        if (shouldProcess) {
            if (rising) {
                constexpr int jumptarget = 1716;
                if (currentLevel < (jumptarget << 16)) {
                    currentLevel = jumptarget << 16;
                }
                currentLevel += (((17 << 24) - currentLevel) >> 24) * increment;
                if (currentLevel >= static_cast<uint32_t>(targetLevel)) {
                    currentLevel = targetLevel;
                    goToState(currentState + 1);
                }
            } else {
                currentLevel -= increment;
                if (currentLevel <= static_cast<uint32_t>(targetLevel)) {
                    currentLevel = targetLevel;
                    goToState(currentState + 1);
                }
            }
        }

        return LUT::exp2(static_cast<float>(currentLevel) * INV_Q24_ONE - 14.0f);
    }
    
    void reset() {
        goToState(4);
        currentLevel = 0;
    }
    
    uint8_t getState() const { return currentState; }

    bool isActive() const {
        return initialised && (currentState < 4 || (currentState == 4 && levels[3] > 0));
    }
};

#ifndef ENVELOPE_STATIC_DEFINED
#define ENVELOPE_STATIC_DEFINED
constexpr uint8_t Envelope::levelLUT[20];
constexpr int Envelope::statics[77];
#endif

#endif // ENVELOPE_H