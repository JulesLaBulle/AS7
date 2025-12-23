#ifndef VOICE_H
#define VOICE_H

#include "operator.h"
#include "algorithm.h"
#include "config.h"
#include "lfo.h"

// -----------------------------------------------------------------------------
// Voice Class
// Represents a single FM synthesis voice (monophonic)
// Manages all operators and algorithm for one note
// -----------------------------------------------------------------------------
class Voice {
private:
    std::array<Operator, NUM_OPERATORS> operators = {};
    Algorithm algorithm = {};
    
    const VoiceConfig* config = nullptr;

    uint8_t currentMidiNote = 0;
    
public:
    Voice() = default;
    
    void configure(const VoiceConfig* voiceConfig) {
        if (!voiceConfig || !voiceConfig->algorithm) {
            return;
        }
        
        config = voiceConfig;
        
        // Clear existing algorithm
        algorithm.resetAll();
        
        // Configure operators
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operators[i].setConfig(&config->operatorConfigs[i]);
        }
        
        // Add operators to algorithm
        for (auto& op : operators) {
            algorithm.addOperator(&op);
        }
        
        // Configure algorithm
        algorithm.setConfig(config->algorithm);
        
        // Set feedback
        algorithm.setFeedback(config->feedback);
        
        // Reset state
        reset();
    }
    
    void updateConfig(const VoiceConfig* voiceConfig) {
        if (!voiceConfig) return;
        
        config = voiceConfig;
        
        // Update operator configurations
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operators[i].setConfig(&config->operatorConfigs[i]);
        }
        
        // Update algorithm configuration
        algorithm.setConfig(config->algorithm);
        
        // Update feedback
        algorithm.setFeedback(config->feedback);
    }
    
    void noteOn(uint8_t midiNote, uint8_t velocity = 100) {
        if (!config) return;
    
        currentMidiNote = midiNote;

        int note = static_cast<int>(midiNote) + static_cast<int>(config->transpose) - 24;   // Apply transpose
        note = std::max(0, std::min(127, note));  // Clamp 0-127 to stay in bounds of uint8_t

        algorithm.triggerAll(static_cast<uint8_t>(note), velocity);
    }
    
    void noteOff() {
        algorithm.releaseAll();
    }

    void setFeedback(uint8_t feedbackValue) {
        algorithm.setFeedback(feedbackValue);
    }
    
    void setAlgorithm(const AlgorithmConfig* algorithmConfig) {
        algorithm.setConfig(algorithmConfig);
    }
    
    inline float process() {        
        return algorithm.process();
    }
    
    // Reset voice to initial state
    void reset() {
        algorithm.resetAll();
        for(size_t i = 0; i < NUM_OPERATORS; ++i) {
            operators[i].reset();
        }
    }

    uint8_t getCurrentMidiNote() const {
        return currentMidiNote;
    }

    bool isActive() {
        for(auto& op : operators) {
            if(op.isActive()) {
                return true;
            }
        }
        return false;
    }

    void setLFO(LFO* lfo) {
        for(auto& op : operators) {
            op.setLFO(lfo);
        }
    }
};

#endif // VOICE_H