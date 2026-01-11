#ifndef PARAMS_H
#define PARAMS_H

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "constants.h"

struct ModWheelAssignment {
    bool pitchModDepth = false;  // Modulate pitch (vibrato)
    bool ampModDepth = false;    // Modulate amplitude (tremolo)
    bool egBias = false;         // Modulate envelope amount (expression)
    
    ModWheelAssignment(bool pitch = false, bool amp = false, bool eg = false)
        : pitchModDepth(pitch), ampModDepth(amp), egBias(eg) {}
};

struct Params {
    // Pitch bend range in semitones (applies to both up and down)
    // Range: 0-24, Default: 12
    uint8_t pitchBendRange = 12;
    
    // Mod wheel intensity (0 = no effect, higher = more modulation)
    // Range: 0-99, Default: 0
    uint8_t modWheelIntensity = 0;
    
    // Mod wheel assignment targets
    ModWheelAssignment modWheelAssignment = ModWheelAssignment();
    
    // MIDI input channel (1-16, 0 = OMNI/all channels)
    // Default: 1
    uint8_t midiChannel = 1;
    
    Params() = default;
    
    Params(uint8_t pbRange, uint8_t mwIntensity, ModWheelAssignment mwAssign, uint8_t midiCh)
        : pitchBendRange(pbRange), modWheelIntensity(mwIntensity), 
          modWheelAssignment(mwAssign), midiChannel(midiCh) {}
    
    // Reset all parameters to their default values
    void setDefaults() {
        pitchBendRange = 12;
        modWheelIntensity = 0;
        modWheelAssignment = ModWheelAssignment(false, false, false);
        midiChannel = 1;
    }
    
    // Load parameters from a file
    bool loadFromFile(const char* filePath = PARAMS_FILE_PATH) {
        FILE* file = fopen(filePath, "rb");
        if (!file) {
            // File doesn't exist, use defaults
            setDefaults();
            return false;
        }
        
        // Read and verify magic number
        uint32_t magic = 0;
        if (fread(&magic, sizeof(magic), 1, file) != 1 || magic != PARAMS_MAGIC) {
            fclose(file);
            setDefaults();
            return false;
        }
        
        // Read and check version
        uint8_t version = 0;
        if (fread(&version, sizeof(version), 1, file) != 1) {
            fclose(file);
            setDefaults();
            return false;
        }
        
        // For now, only support current version
        // Future versions can add migration logic here
        if (version != PARAMS_VERSION) {
            fclose(file);
            setDefaults();
            return false;
        }
        
        // Read parameters
        bool success = true;
        success &= (fread(&pitchBendRange, sizeof(pitchBendRange), 1, file) == 1);
        success &= (fread(&modWheelIntensity, sizeof(modWheelIntensity), 1, file) == 1);
        success &= (fread(&modWheelAssignment.pitchModDepth, sizeof(bool), 1, file) == 1);
        success &= (fread(&modWheelAssignment.ampModDepth, sizeof(bool), 1, file) == 1);
        success &= (fread(&modWheelAssignment.egBias, sizeof(bool), 1, file) == 1);
        success &= (fread(&midiChannel, sizeof(midiChannel), 1, file) == 1);
        
        fclose(file);
        
        if (!success) {
            setDefaults();
            return false;
        }
        
        // Validate loaded values
        validateAndClamp();
        
        return true;
    }
    
    // Save current parameters to a file
    bool saveToFile(const char* filePath = PARAMS_FILE_PATH) const {
        FILE* file = fopen(filePath, "wb");
        if (!file) {
            return false;
        }
        
        bool success = true;
        
        // Write magic number
        uint32_t magic = PARAMS_MAGIC;
        success &= (fwrite(&magic, sizeof(magic), 1, file) == 1);
        
        // Write version
        uint8_t version = PARAMS_VERSION;
        success &= (fwrite(&version, sizeof(version), 1, file) == 1);
        
        // Write parameters
        success &= (fwrite(&pitchBendRange, sizeof(pitchBendRange), 1, file) == 1);
        success &= (fwrite(&modWheelIntensity, sizeof(modWheelIntensity), 1, file) == 1);
        success &= (fwrite(&modWheelAssignment.pitchModDepth, sizeof(bool), 1, file) == 1);
        success &= (fwrite(&modWheelAssignment.ampModDepth, sizeof(bool), 1, file) == 1);
        success &= (fwrite(&modWheelAssignment.egBias, sizeof(bool), 1, file) == 1);
        success &= (fwrite(&midiChannel, sizeof(midiChannel), 1, file) == 1);
        
        fclose(file);
        
        return success;
    }
    
    // Validate and clamp all parameter values to valid ranges
    void validateAndClamp() {
        // Pitch bend range: 0-24 semitones
        if (pitchBendRange > 24) pitchBendRange = 24;
        
        // Mod wheel intensity: 0-99
        if (modWheelIntensity > 99) modWheelIntensity = 99;
        
        // MIDI channel: 0-16 (0 = OMNI)
        if (midiChannel > 16) midiChannel = 16;
    }
    
    // Print current parameters (for debugging)
    void print() const {
        printf("=== GLOBAL PARAMETERS ===\n");
        printf("Pitch Bend Range: %d semitones\n", pitchBendRange);
        printf("Mod Wheel Intensity: %d\n", modWheelIntensity);
        printf("Mod Wheel Assignment:\n");
        printf("  - Pitch Mod Depth: %s\n", modWheelAssignment.pitchModDepth ? "ON" : "OFF");
        printf("  - Amp Mod Depth: %s\n", modWheelAssignment.ampModDepth ? "ON" : "OFF");
        printf("  - EG Bias: %s\n", modWheelAssignment.egBias ? "ON" : "OFF");
        printf("MIDI Channel: %d%s\n", midiChannel, midiChannel == 0 ? " (OMNI)" : "");
        printf("=========================\n");
    }
};

#endif // PARAMS_H