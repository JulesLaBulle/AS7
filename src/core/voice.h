#ifndef VOICE_H
#define VOICE_H

#include "operator.h"
#include "algorithm.h"
#include "config.h"
#include "lfo.h"
#include "pitchenv.h"

// Single FM voice (monophonic) - manages 6 operators + algorithm
class Voice {
private:
    std::array<Operator, NUM_OPERATORS> operators = {};
    Algorithm algorithm = {};
    PitchEnvelope pitchEnv = {};
    
    const VoiceConfig* config = nullptr;
    LFO* lfo = nullptr;
    uint8_t currentMidiNote = 0;
    
public:
    Voice() = default;
    
    void configure(const VoiceConfig* voiceConfig) {
        if (!voiceConfig || !voiceConfig->algorithm) return;
        
        config = voiceConfig;
        algorithm.resetAll();
        
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operators[i].setConfig(&config->operatorConfigs[i]);
        }
        
        for (auto& op : operators) {
            algorithm.addOperator(&op);
        }
        
        algorithm.setConfig(config->algorithm);
        algorithm.setFeedback(config->feedback);
        reset();
    }

    void setPitchEnvelopeConfig(const PitchEnvelopeConfig* peConfig) {
        pitchEnv.setConfig(peConfig);
    }

    void setLFO(LFO* lfoPtr) {
        lfo = lfoPtr;
    }
    
    void updateConfig(const VoiceConfig* voiceConfig) {
        if (!voiceConfig) return;
        config = voiceConfig;
        
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operators[i].setConfig(&config->operatorConfigs[i]);
        }
        
        algorithm.setConfig(config->algorithm);
        algorithm.setFeedback(config->feedback);
    }

    void setFeedback(uint8_t feedbackValue) {
        algorithm.setFeedback(feedbackValue);
    }
    
    void setAlgorithm(const AlgorithmConfig* algorithmConfig) {
        algorithm.setConfig(algorithmConfig);
    }

    void setOSCKeySync(bool sync) {
        if (!config) return;
        for (size_t i = 0; i < NUM_OPERATORS; i++) {
            const_cast<OperatorConfig*>(&config->operatorConfigs[i])->OSCKeySync = sync;
        }
    }
    
    void noteOn(uint8_t midiNote, uint8_t velocity = 100) {
        if (!config) return;
    
        currentMidiNote = midiNote;

        int note = static_cast<int>(midiNote) + static_cast<int>(config->transpose) - 24;
        if (note < 0) note = 0;
        else if (note > 127) note = 127;

        algorithm.triggerAll(static_cast<uint8_t>(note), velocity);
        pitchEnv.trigger();
    }
    
    void noteOff() {
        algorithm.releaseAll();
        pitchEnv.release();
    }

    // Process one sample - optimized hot path
    inline float process() {
        float pitchMod = pitchEnv.process();
        float ampMod = 0.0f;
        
        if (lfo) {
            pitchMod *= lfo->getPitchMod();
            ampMod = lfo->getAmpMod();
        }
        
        return algorithm.process(pitchMod, ampMod);
    }
    
    void reset() {
        algorithm.resetAll();
        pitchEnv.reset();
        for (auto& op : operators) {
            op.reset();
        }
    }

    uint8_t getCurrentMidiNote() const { return currentMidiNote; }

    bool isActive() const {
        for (const auto& op : operators) {
            if (op.isActive()) return true;
        }
        return false;
    }
};

#endif // VOICE_H