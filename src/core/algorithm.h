#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "operator.h"
#include "config.h"
#include <array>

// FM algorithm: routes 6 operators with modulation matrix
class Algorithm {
private:
    std::array<Operator*, NUM_OPERATORS> operators = {nullptr};
    float modulationBuffer[NUM_OPERATORS] = {0.0f};
    const AlgorithmConfig* config = nullptr;
    
public:
    bool addOperator(Operator* op) {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i] == nullptr) {
                operators[i] = op;
                return true;
            }
        }
        return false;
    }
    
    void setConfig(const AlgorithmConfig* algConfig) {
        config = algConfig;
    }
    
    void setFeedback(uint8_t feedbackValue) {
        if (!config) return;
        if (config->hasFeedback && config->feedbackOperator < NUM_OPERATORS) {
            operators[config->feedbackOperator]->setFeedback(feedbackValue);
        }
    }

    void triggerAll(uint8_t midiNote, uint8_t velocity = 100) {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) operators[i]->trigger(midiNote, velocity);
        }
    }
    
    void releaseAll() {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) operators[i]->release();
        }
    }
    
    void resetAll() {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) operators[i]->reset();
            modulationBuffer[i] = 0.0f;
        }
    }
    
    // Process one sample - optimized hot path
    inline float process(float pitchMod, float ampMod) {
        if (!config) return 0.0f;

        float finalOutput = 0.0f;

        // Clear modulation buffer
        modulationBuffer[0] = 0.0f;
        modulationBuffer[1] = 0.0f;
        modulationBuffer[2] = 0.0f;
        modulationBuffer[3] = 0.0f;
        modulationBuffer[4] = 0.0f;
        modulationBuffer[5] = 0.0f;

        // Process from highest to lowest index (5 to 0)
        for (int i = NUM_OPERATORS - 1; i >= 0; --i) {
            Operator* op = operators[i];
            if (!op) continue;

            // Accumulate phase modulation from modulators
            float phaseMod = 0.0f;
            const int modCount = config->modulatorCount[i];
            for (int j = 0; j < modCount; ++j) {
                phaseMod += modulationBuffer[config->modulatorIndices[i][j]];
            }
            phaseMod *= MODULATION_SCALING;

            float output;
            if (config->hasFeedback && i == config->feedbackOperator) {
                output = op->processWithFeedback(pitchMod, ampMod);
            } else {
                output = op->process(phaseMod, pitchMod, ampMod);
            }

            modulationBuffer[i] = output;

            if (config->isCarrier[i]) {
                finalOutput += output;
            }
        }

        return finalOutput;
    }
};

#endif // ALGORITHM_H