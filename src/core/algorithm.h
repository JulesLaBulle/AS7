#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "operator.h"
#include "config.h"
#include <array>

class Algorithm {
private:
    // Array of operator pointers (fixed size for performance)
    std::array<Operator*, NUM_OPERATORS> operators = {nullptr};
    size_t numOperators = 0;
    
    // Modulation buffer (reused each sample to avoid allocation)
    float modulationBuffer[NUM_OPERATORS] = {0.0f};
    
    // Configuration reference (modifiable in real-time)
    const AlgorithmConfig* config = nullptr;
    
    
public:
    // -------------------------------------------------------------------------
    // Configuration Methods
    // -------------------------------------------------------------------------
    
    // Add an operator to the algorithm
    bool addOperator(Operator* op) {
        if (numOperators >= NUM_OPERATORS) {
            return false;
        }
        operators[numOperators] = op;
        ++numOperators;
        return true;
    }
    
    // Set algorithm configuration (can be called in real-time)
    void setConfig(const AlgorithmConfig* algConfig) {
        config = algConfig;
    }
    
    // Set feedback level (0-7, DX7 style)
    void setFeedback(uint8_t feedbackValue) {
        if (!config) return;
        
        // Apply feedback to the designated operator (if any)
        if (config->hasFeedback && config->feedbackOperator < numOperators) {
            operators[config->feedbackOperator]->setFeedback(feedbackValue);
        }
    }

    // -------------------------------------------------------------------------
    // Runtime Control Methods
    // -------------------------------------------------------------------------
    
    // Trigger all operators (start note)
    void triggerAll(uint8_t midiNode, uint8_t velocity = 100) {

        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) {
                operators[i]->trigger(midiNode, velocity);
            }
        }
    }
    
    // Release all operators (stop note)
    void releaseAll() {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) {
                operators[i]->release();
            }
        }
    }
    
    // Reset all operators to initial state
    void resetAll() {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            if (operators[i]) {
                operators[i]->reset();
            }
        }
        
        // Clear modulation buffer
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            modulationBuffer[i] = 0.0f;
        }
    }
    
    // -------------------------------------------------------------------------
    // Audio Processing Methods
    // -------------------------------------------------------------------------
    
    // Process one audio sample (highly optimized for real-time)
    inline float process() {
        if(!config) return 0.0f;

        float finalOutput = 0.0f;

        for(size_t i = 0; i < NUM_OPERATORS; ++i) {
            modulationBuffer[i] = 0.0f;
        }

        // Process from highest to lowest index (5 to 0)
        for(int i = NUM_OPERATORS - 1; i >= 0; --i) {

            if(!operators[i]) continue;

            // Calculate modulation for this operator
            float phaseMod = 0.0f;
            for(int j = 0; j < config->modulatorCount[i]; ++j) {
                uint8_t modIndex = config->modulatorIndices[i][j];
                phaseMod += modulationBuffer[modIndex];
            }

            float output = 0.0f;

            // Special handling for feedback operator
            if(config->hasFeedback && i == config->feedbackOperator) {
                // Feedback operator uses its own previous output
                output = operators[i]->processWithFeedback();
            } else {
                // Regular operator (modulator or carrier)
                output = operators[i]->process(phaseMod * MODULATION_SCALING);
            }

            // Store output for lower-index operators
            modulationBuffer[i] = output;

            // Add to final output if carrier
            if(config->isCarrier[i]) {
                finalOutput += output;
            }
        }

        return finalOutput;
    }
};

#endif // ALGORITHM_H