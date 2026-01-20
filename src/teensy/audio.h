#ifndef AUDIO_H
#define AUDIO_H

#include <Audio.h>
#include "../core/synth.h"

// Audio output stream - generates samples from synthesizer
class AudioOutput : public AudioStream {
private:
    Synth* synth;
    float volume = 0.8f;

public:
    AudioOutput(Synth* synthPtr) : AudioStream(0, nullptr), synth(synthPtr) {}

    // Generate a samples buffer
    virtual void update(void) override {
        audio_block_t* block = allocate();
        if (!block) return;

        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            float sample = synth->process() * volume;
            
            // Clamp + convert to int16
            if (sample > 1.0f) sample = 1.0f;
            else if (sample < -1.0f) sample = -1.0f;
            
            block->data[i] = (int16_t)(sample * 32767.0f);
        }

        transmit(block);
        release(block);
    }

    void setVolume(float v) {
        volume = (v > 1.0f) ? 1.0f : (v < 0.0f) ? 0.0f : v;
    }

    float getVolume() const {
        return volume;
    }
};

// Audio manager - handles initialization and configuration
namespace Audio {
    static AudioOutput* output = nullptr;              // Synthesizer output stream
    static AudioOutputI2S i2s_out;                     // I2S interface (DAC communication)
    static AudioControlSGTL5000 sgtl5000;              // Codec controller (volume, etc)

    // Initialize audio system and shield
    bool init(Synth* synth) {
        if (!synth) return false;
        if (output) return true; // Already initialized

        // Allocate 120 blocks of memory for audio processing
        // Each block = 128 samples @ 44.1kHz = 2.9ms
        // Total = ~348ms buffer (latency vs stability tradeoff)
        // Increase for stability, decrease for lower latency
        AudioMemory(120);
        
        // Create audio output generator connected to synth
        output = new AudioOutput(synth);
        
        // Connect mono output to I2S stereo (duplicate left=right)
        // Output channel 0 → I2S left channel
        new AudioConnection(*output, 0, i2s_out, 0);
        // Output channel 0 → I2S right channel  
        new AudioConnection(*output, 0, i2s_out, 1);

        // Enable SGTL5000 codec (power on DAC/analog circuits)
        sgtl5000.enable();
        
        // Set line output level (range 0-31)
        // Controls both minijack and line-out connectors
        sgtl5000.lineOutLevel(31);
        
        // Set initial codec volume (0.0 = mute, 1.0 = max)
        sgtl5000.volume(0.8f);

        return true;
    }

    void setVolume(float volume) {
        if (output) output->setVolume(volume);
    }

    float getVolume() {
        return output ? output->getVolume() : 0.0f;
    }
}

#endif // AUDIO_H
