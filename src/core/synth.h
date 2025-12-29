#ifndef SYNTH_H
#define SYNTH_H

#include "constants.h"
#include "config.h"
#include "voice.h"
#include "lfo.h"

// Enable debug output (comment out for production/Teensy)
// #define SYNTH_DEBUG

// Polyphonic FM synthesizer
class Synth {
private:
    std::array<Voice, POLYPHONY> voices = {};
    std::array<uint64_t, POLYPHONY> voiceAge = {0}; 
    uint64_t globalAgeCounter = 0;
    int activeNoteCount = 0;

    LFO lfo = {};

public:
    const SynthConfig* config = nullptr;  // Pointeur const vers config (adresse fixe)
    Synth() = default;

    void configure(const SynthConfig* synthConfigPtr) {
        config = synthConfigPtr;  // Pointeur const, pas de cast nécessaire
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

#ifdef SYNTH_DEBUG
        if (sample >= 1.0f) {
            // Teensy: Serial.println(sample);
            // PC: std::cout << "Clipping: " << sample << std::endl;
        }
#endif

        return sample;
    }

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
};

#endif // SYNTH_H
