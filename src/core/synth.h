#ifndef SYNTH_H
#define SYNTH_H

#include "constants.h"
#include "config.h"
#include "voice.h"
#include "lfo.h"

class Synth {
private:
    std::array<Voice,  POLYPHONY> voices = {};
    std::array<uint64_t, POLYPHONY> voiceAge = {0}; 
    uint64_t globalAgeCounter = 0;
    int activeNoteCount = 0;

    LFO lfo = LFO();
    
    // Pitch envelope

    const SynthConfig* config = nullptr;

public:
    Synth() = default;

    // Configure all voices with a VoiceConfig pointer
    void configure(const SynthConfig* synthConfigPtr) {
        config = synthConfigPtr;

        lfo.configure(&config->lfoConfig);
        for (auto& voice : voices) {
            voice.configure(&config->voiceConfig);
            voice.setLFO(&lfo);
        }
    }

    // Note On: find a free voice and start the note
    void noteOn(uint8_t midiNote, uint8_t velocity = 100) {
        if(!config) return; 

        // Monophonic mode: steal the only voice
        if(config->monophonic) {
            if(voices[0].isActive()) {
                // TODO: Portamento here
            }

            for(auto& voice : voices) {
                voice.noteOff();
            }

            lfo.trigger();
            voices[0].noteOn(midiNote, velocity);
            return;
        }

        // Try to find a free voice
        for (size_t i = 0; i < POLYPHONY; ++i) {
            if(!voices[i].isActive()) {
                voiceAge[i] = globalAgeCounter++;
                voices[i].noteOn(midiNote, velocity);

                activeNoteCount++;
                if(activeNoteCount == 1 || config->lfoConfig.LFOKeySync) {  // First active voice triggers the LFO or LFO Key Sync enabled
                    lfo.trigger();
                }

                return;
            }
        }

        // If no free voice, steal the oldest one
        size_t oldestIndex = 0;
        uint64_t oldestAge = voiceAge[0];

        for (size_t i = 0; i < POLYPHONY; ++i) {
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
        if(!config) return; 

        // Monophonic mode: stop the only voice
        if(config->monophonic) {
            voices[0].noteOff();
            return;
        }

        // Find the voice playing this note and stop it
        for (auto& voice : voices) {
            if (voice.isActive() && voice.getCurrentMidiNote() == midiNote) {
                voice.noteOff();
                activeNoteCount--;
                return;
            }
        }
    }

    inline float process() {
        if(!config) return 0.0f; 

        lfo.process();

        float sample = 0.0f;
        for (auto& voice : voices) {
            if(voice.isActive()) {
                sample += voice.process();
            }
            
        }

        //if(sample >= 1.0f) std::cout << "Clipping: " << sample << std::endl; // TEENSY: replace with Serial.println
        // TODO: implement soft clipping with tanh (LUT) or similar

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
};

#endif // SYNTH_H