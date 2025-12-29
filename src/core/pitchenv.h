#ifndef PITCHENV_H
#define PITCHENV_H

#include "constants.h"
#include "config.h"
#include "lut.h"

// Pitch envelope: returns frequency multiplier (1.0 = no change)
// Internally works in Q24 log domain (like Dexed), converts to float on output
class PitchEnvelope {
private:
    const PitchEnvelopeConfig* config = nullptr;

    // Unit increment per sample (from Dexed: N * (1 << 24) / (21.3 * sample_rate))
    static constexpr float UNIT_BASE = 16777216.0f / (21.3f * 44100.0f);
    static constexpr float UNIT = UNIT_BASE * (44100.0f / SAMPLE_RATE);

    // State (all Q24 format)
    int32_t level = 0;
    int32_t targetLevel = 0;
    int32_t increment = 0;
    uint8_t stage = 3;
    bool rising = false;
    bool keyDown = false;

    void advanceStage(uint8_t newStage) {
        stage = newStage;
        if (stage >= 4) return;

        uint8_t rate, levelParam;
        switch (stage) {
            case 0: rate = config->r1; levelParam = config->l1; break;
            case 1: rate = config->r2; levelParam = config->l2; break;
            case 2: rate = config->r3; levelParam = config->l3; break;
            default: rate = config->r4; levelParam = config->l4; break;
        }

        targetLevel = static_cast<int32_t>(PITCHENV_TAB[levelParam]) << 19;
        rising = (targetLevel > level);
        increment = static_cast<int32_t>(PITCHENV_RATE[rate] * UNIT);
    }

public:
    PitchEnvelope() = default;

    void setConfig(const PitchEnvelopeConfig* pitchEnvConfig) {
        config = pitchEnvConfig;
        if (config) {
            level = static_cast<int32_t>(PITCHENV_TAB[config->l4]) << 19;
            keyDown = false;
            stage = 3;
        }
    }

    void trigger() {
        if (!config) return;
        keyDown = true;
        advanceStage(0);
    }

    void release() {
        if (!config) return;
        keyDown = false;
        advanceStage(3);
    }

    // Process one sample, returns frequency multiplier
    inline float process() {
        if (!config) return 1.0f;

        // Process stages 0-2 always, stage 3 only on release
        const bool shouldProcess = (stage < 3) || (stage == 3 && !keyDown);
        
        if (shouldProcess) {
            if (rising) {
                level += increment;
                if (level >= targetLevel) {
                    level = targetLevel;
                    if (stage < 3) advanceStage(stage + 1);
                }
            } else {
                level -= increment;
                if (level <= targetLevel) {
                    level = targetLevel;
                    if (stage < 3) advanceStage(stage + 1);
                }
            }
        }

        return LUT::exp2(static_cast<float>(level) * INV_Q24_ONE);
    }

    void reset() {
        if (config) {
            level = static_cast<int32_t>(PITCHENV_TAB[config->l4]) << 19;
        } else {
            level = 0;
        }
        stage = 3;
        keyDown = false;
    }
};

#endif // PITCHENV_H
