#ifndef SYNTH_H
#define SYNTH_H

#include "constants.h"
#include "config.h"
#include "voice.h"
#include "lfo.h"
#include "params.h"

// Forward declaration
class MidiHandler;

// Polyphonic FM synthesizer
class Synth {
private:
    std::array<Voice, POLYPHONY> voices = {};
    std::array<uint64_t, POLYPHONY> voiceAge = {0}; 
    uint64_t globalAgeCounter = 0;
    int activeNoteCount = 0;

    LFO lfo = {};

    Params params = {};
    
    MidiHandler* midiHandler = nullptr;

public:
    const SynthConfig* config = nullptr;
    Synth() = default;
    
    // Parameters management
    bool initParams(const char* filePath = PARAMS_FILE_PATH) {
        return params.loadFromFile(filePath);
    }

    bool saveParams(const char* filePath = PARAMS_FILE_PATH) const {
        return params.saveToFile(filePath);
    }
    
    void setPitchBendRange(uint8_t semitones) {
        params.pitchBendRange = (semitones > 24) ? 24 : semitones;
    }
    
    void setModWheelIntensity(uint8_t intensity) {
        params.modWheelIntensity = (intensity > 99) ? 99 : intensity;
    }
    
    void setModWheelAssignment(bool pitchMod, bool ampMod, bool egBias) {
        params.modWheelAssignment.pitchModDepth = pitchMod;
        params.modWheelAssignment.ampModDepth = ampMod;
        params.modWheelAssignment.egBias = egBias;
    }
    
    void setMidiChannel(uint8_t channel) {
        params.midiChannel = (channel > 16) ? 16 : channel;
        // Update MidiHandler if registered
        if (midiHandler) {
            updateMidiHandlerChannel();
        }
    }
    
    uint8_t getMidiChannel() const {
        return params.midiChannel;
    }
    
    void setMidiHandler(MidiHandler* handler) {
        midiHandler = handler;
    }
    
    void updateMidiHandlerChannel();

    void printParams() const {
        params.print();
    }

    // Config management
    void setFeedback(uint8_t feedback) {
        for (auto& voice : voices) {
            voice.setFeedback(feedback);
        }
    }

    void setAlgorithm(const AlgorithmConfig* algorithmConfig) {
        for (auto& voice : voices) {
            voice.setAlgorithm(algorithmConfig);
        }
    }

    void setOSCKeySync(bool sync) {
        for (auto& voice : voices) {
            voice.setOSCKeySync(sync);
        }
    }

    // Synth configuration
    void configure(const SynthConfig* synthConfigPtr) {
        config = synthConfigPtr;
        lfo.configure(&config->lfoConfig);
        
        for (auto& voice : voices) {
            voice.configure(&config->voiceConfig);
            voice.setPitchEnvelopeConfig(&config->pitchEnvelopeConfig);
            voice.setLFO(&lfo);
        }
    }

    void noteOn(uint8_t midiNote, uint8_t velocity = 100) {
        if (!config) return; 

        // Monophonic mode
        if (config->monophonic) {
            for (auto& voice : voices) {
                voice.noteOff();
            }
            lfo.trigger();
            voices[0].noteOn(midiNote, velocity);
            return;
        }

        // Find free voice
        for (size_t i = 0; i < POLYPHONY; ++i) {
            if (!voices[i].isActive()) {
                voiceAge[i] = globalAgeCounter++;
                voices[i].noteOn(midiNote, velocity);
                ++activeNoteCount;
                
                if (activeNoteCount == 1 || config->lfoConfig.LFOKeySync) {
                    lfo.trigger();
                }
                return;
            }
        }

        // Voice stealing: find oldest
        size_t oldestIndex = 0;
        uint64_t oldestAge = voiceAge[0];
        
        for (size_t i = 1; i < POLYPHONY; ++i) {
            if (voiceAge[i] < oldestAge) {
                oldestAge = voiceAge[i];
                oldestIndex = i;
            }
        }

        voices[oldestIndex].noteOff();
        voiceAge[oldestIndex] = globalAgeCounter++;
        voices[oldestIndex].noteOn(midiNote, velocity);
    }

    void noteOff(uint8_t midiNote) {
        if (!config) return; 

        if (config->monophonic) {
            voices[0].noteOff();
            return;
        }

        for (auto& voice : voices) {
            if (voice.isActive() && voice.getCurrentMidiNote() == midiNote) {
                voice.noteOff();
                --activeNoteCount;
                return;
            }
        }
    }

    // Process one sample - optimized hot path
    inline float process() {
        if (!config) return 0.0f; 

        lfo.process();

        float sample = 0.0f;
        for (auto& voice : voices) {
            if (voice.isActive()) {
                sample += voice.process();
            }
        }

        #ifdef DEBUG_PC
        if(sample > 1.0f || sample < -1.0f) {
            std::cout << "CLIPPING: " << sample << std::endl;
        }
        #endif
        #ifdef DEBUG_TEENSY
        if(sample > 1.0f || sample < -1.0f) {
            Serial.print(F("CLIPPING: ")); 
            Serial.println(sample);
        }
        #endif

        return sample;
    }
};

// Include MidiHandler for implementation
#ifdef PLATFORM_TEENSY
#include "../teensy/hardware/midi.h"

inline void Synth::updateMidiHandlerChannel() {
    if (midiHandler) {
        midiHandler->updateChannel();
    }
}
#else
inline void Synth::updateMidiHandlerChannel() {
    // No-op on PC platform
}
#endif

#endif // SYNTH_H
