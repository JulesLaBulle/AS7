#ifndef PITCHENV_H
#define PITCHENV_H

#include "constants.h"
#include "config.h"
#include "lut.h"

// Pitch envelope: returns frequency multiplier (1.0 = no change)
// Similar to Envelope which returns amplitude multiplier
// Internally works in Q24 log domain (like Dexed), but converts to float multiplier on output
class PitchEnvelope {
private:
    const PitchEnvelopeConfig* config = nullptr;

    // Unit increment per sample (from Dexed: N * (1 << 24) / (21.3 * sample_rate))
    // For sample-by-sample processing (N=1): (1 << 24) / (21.3 * 44100)
    // Scaled for any sample rate
    static constexpr float UNIT_BASE = 16777216.0f / (21.3f * 44100.0f);  // ~17.85
    static constexpr float UNIT = UNIT_BASE * (44100.0f / SAMPLE_RATE);

    // Internal state - all in Q24 format (like Dexed)
    int32_t level = 0;          // Current level: PITCHENV_TAB[...] << 19
    int32_t targetLevel = 0;    // Target level for current stage
    int32_t increment = 0;      // Increment per sample in Q24
    bool rising = false;
    uint8_t stage = 3;          // Current stage (0-3)
    bool keyDown = false;

    // Advance to next stage
    void advanceStage(uint8_t newStage) {
        stage = newStage;

        if (stage < 4) {
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
    }

public:
    PitchEnvelope() = default;

    void setConfig(const PitchEnvelopeConfig* pitchEnvConfig) {
        config = pitchEnvConfig;
        if (config) {
            // Initialize at L4 level (release level)
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

    // Process one sample
    // Returns frequency multiplier (like Envelope returns amplitude multiplier)
    // 1.0 = no change, 2.0 = octave up, 0.5 = octave down
    inline float process() {
        if (!config) return 1.0f;

        // Same logic as Dexed: process stages 0-2 always, stage 3 only if key is released (!keyDown)
        if (stage < 3 || (stage == 3 && !keyDown)) {
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

        // Convert Q24 level to frequency multiplier (like Envelope converts to amplitude multiplier)
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
