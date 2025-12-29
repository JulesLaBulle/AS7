#include <iostream>
#include <vector>
#include <chrono>

#include "core/config.h"
#include "core/connections.h"
#include "core/constants.h"
#include "core/lut.h"
#include "core/synth.h"
#include "core/voice.h"
#include "core/lfo.h"
#include "core/sysex.h"

#include "pc/wav_writer.h"

// Test parameters
constexpr char FILE_NAME[] = "fm_synth.wav";
constexpr char BANK_FILE_PATH[] = "./presets/rom1a.syx";
constexpr uint8_t PRESET_NUMBER = 15; // 0-31

constexpr float NOTE_DURATION = 8.0f;   
constexpr float TOTAL_DURATION = 10.0f;
constexpr size_t TOTAL_SAMPLES = static_cast<size_t>(SAMPLE_RATE * TOTAL_DURATION);

int main() {
    // Initialize Look Up Tables
    LUT::init();

    // -------------------------------------------------------------------------
    // Create and configure the voice
    // -------------------------------------------------------------------------
    Synth synth = Synth();
    
    /*
    OperatorConfig opConfigs[6] = {
        OperatorConfig(
            true,           // on/off
            FrequencyConfig(false, 10, 1, 0),                       // Fixed, Detune, Coarse, Fine
            EnvelopeConfig(99, 99, 75, 0,  0, 96, 25, 25, 76, 3),   // OutputLvl, L1,L2,L3,L4, R1,R2,R3,R4, RateScaling
            2,              // Vel Sens
            3               // AmpModSens
        ),
        OperatorConfig(
            true, 
            FrequencyConfig(false, 7, 14, 0), 
            EnvelopeConfig(58, 99, 75, 0,  0, 95, 50, 35, 78, 3),
            7, 
            3
        ),
        OperatorConfig(
            true, 
            FrequencyConfig(false, 7, 1, 0), 
            EnvelopeConfig(99, 99, 95, 0, 0, 95, 20, 20, 50, 3), 
            2, 
            3
        ),
        OperatorConfig(
            true, 
            FrequencyConfig(false, 7, 1, 0), 
            EnvelopeConfig(89, 99, 95, 0, 0, 95, 29, 20, 50, 3), 
            6, 
            3
        ),
        OperatorConfig(
            true, 
            FrequencyConfig(false, 0, 1, 0), 
            EnvelopeConfig(99, 99, 95, 0, 0, 95, 20, 20, 50, 3), 
            0, 
            3
        ),
        OperatorConfig(
            true, 
            FrequencyConfig(false, 14, 1, 0), 
            EnvelopeConfig(79, 99, 95, 0, 0, 95, 29, 20, 50, 3), 
            6, 
            3
        )
    };

    SynthConfig synthConfig(
        VoiceConfig(
            opConfigs,
            Algorithms::ALL_ALGORITHMS[4],
            6,              // Feedback
            24              // Transpose
        ), 
        LFOConfig(4,        // Waveform 
                  34,       // Speed
                  33,       // Delay
                  0,        // Pitch Mod Depth
                  0,        // Amp Mod Depth
                  0,        // Pitch Mod Sens
                  false   // LFO Key Sync 
        ),
        PitchEnvelopeConfig(
            50, 50, 50, 50,   // L1, L2, L3, L4
            0, 0, 0, 0        // R1, R2, R3, R4
        ),
        false               // Monophonic
    );

    synth.configure(&synthConfig);
    */
    
    SynthConfig presetConfig;

    SysexHandler sysex;
    if (sysex.loadBank(BANK_FILE_PATH)) {
        std::cout << "DEBUG: Bank loaded, now loading preset..." << std::endl;
        
        if (sysex.loadPreset(&presetConfig, PRESET_NUMBER)) {
            synth.configure(&presetConfig);
            std::cout << "Loaded preset: " << sysex.getPresetName(PRESET_NUMBER) << std::endl;
        }
    }
    printSynthConfig(presetConfig);
    
    // -------------------------------------------------------------------------
    // Generate audio
    // -------------------------------------------------------------------------
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::vector<float> samples;
    samples.reserve(TOTAL_SAMPLES);
    
    // Play notes
    synth.noteOn(69, 100);
    
    for (size_t i = 0; i < TOTAL_SAMPLES; ++i) {
        if (i == static_cast<size_t>(SAMPLE_RATE * 1.0f)) {
            synth.noteOn(72, 100);
        }

        if (i == static_cast<size_t>(SAMPLE_RATE * 2.0f)) {
            synth.noteOn(76, 100);
        }

        // Release note after NOTE_DURATION seconds
        if (i == static_cast<size_t>(SAMPLE_RATE * NOTE_DURATION)) {
            synth.noteOff(69);
            synth.noteOff(72);
            synth.noteOff(76);
        }

        // Process one sample
        samples.push_back(synth.process());
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    float timeSeconds = static_cast<float>(duration.count()) / 1000000.0f;
    
    // -------------------------------------------------------------------------
    // Write WAV file and display statistics
    // -------------------------------------------------------------------------
    if (WavWriter::writeFile(FILE_NAME, samples, static_cast<uint32_t>(SAMPLE_RATE))) {
        std::cout << "=== AS7 Test ===\n";
        std::cout << "Samples generated: " << samples.size() << "\n";
        std::cout << "Total duration: " << TOTAL_DURATION << " seconds\n";
        std::cout << "Generation time: " << duration.count() << " µs\n";
        std::cout << "Real-time factor: " << (TOTAL_DURATION / timeSeconds) << "x\n";
        std::cout << "Effective sample rate: " 
                  << (static_cast<float>(samples.size()) / timeSeconds) << " samples/sec\n";
        
        return 0;
    } else {
        std::cerr << "ERROR: Failed to create WAV file\n";
        return 1;
    }
}