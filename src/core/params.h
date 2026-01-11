#ifndef PARAMS_H
#define PARAMS_H

#include <cstdint>

#ifdef PLATFORM_TEENSY
    #include <SD.h>
#else
    #include <cstdio>
#endif

#ifdef DEBUG_PC
#include <iostream>
#endif

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
        #ifdef PLATFORM_TEENSY
        // Teensy: Read from SD card
        File file = SD.open(filePath);
        if (!file) {
            setDefaults();
            return false;
        }
        
        // Read and verify magic number
        uint32_t magic = 0;
        if (file.read((uint8_t*)&magic, sizeof(magic)) != sizeof(magic) || magic != PARAMS_MAGIC) {
            file.close();
            setDefaults();
            return false;
        }
        
        // Read and check version
        uint8_t version = 0;
        if (file.read(&version, sizeof(version)) != sizeof(version)) {
            file.close();
            setDefaults();
            return false;
        }
        
        if (version != PARAMS_VERSION) {
            file.close();
            setDefaults();
            return false;
        }
        
        // Read parameters
        bool success = true;
        success &= (file.read((uint8_t*)&pitchBendRange, sizeof(pitchBendRange)) == sizeof(pitchBendRange));
        success &= (file.read((uint8_t*)&modWheelIntensity, sizeof(modWheelIntensity)) == sizeof(modWheelIntensity));
        success &= (file.read((uint8_t*)&modWheelAssignment.pitchModDepth, sizeof(bool)) == sizeof(bool));
        success &= (file.read((uint8_t*)&modWheelAssignment.ampModDepth, sizeof(bool)) == sizeof(bool));
        success &= (file.read((uint8_t*)&modWheelAssignment.egBias, sizeof(bool)) == sizeof(bool));
        success &= (file.read((uint8_t*)&midiChannel, sizeof(midiChannel)) == sizeof(midiChannel));
        
        file.close();
        
        #else
        // PC: Read from filesystem
        FILE* file = fopen(filePath, "rb");
        if (!file) {
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
        #endif
        
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
        #ifdef PLATFORM_TEENSY
        // Teensy: Write to SD card
        if (SD.exists(filePath)) {
            SD.remove(filePath);
        }
        File file = SD.open(filePath, FILE_WRITE);
        if (!file) {
            return false;
        }
        
        bool success = true;
        
        // Write magic number
        uint32_t magic = PARAMS_MAGIC;
        success &= (file.write((uint8_t*)&magic, sizeof(magic)) == sizeof(magic));
        
        // Write version
        uint8_t version = PARAMS_VERSION;
        success &= (file.write(&version, sizeof(version)) == sizeof(version));
        
        // Write parameters
        success &= (file.write((uint8_t*)&pitchBendRange, sizeof(pitchBendRange)) == sizeof(pitchBendRange));
        success &= (file.write((uint8_t*)&modWheelIntensity, sizeof(modWheelIntensity)) == sizeof(modWheelIntensity));
        success &= (file.write((uint8_t*)&modWheelAssignment.pitchModDepth, sizeof(bool)) == sizeof(bool));
        success &= (file.write((uint8_t*)&modWheelAssignment.ampModDepth, sizeof(bool)) == sizeof(bool));
        success &= (file.write((uint8_t*)&modWheelAssignment.egBias, sizeof(bool)) == sizeof(bool));
        success &= (file.write((uint8_t*)&midiChannel, sizeof(midiChannel)) == sizeof(midiChannel));
        
        file.close();
        
        #else
        // PC: Write to filesystem
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
        #endif
        
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
        #ifdef DEBUG_PC
        std::cout << "=== GLOBAL PARAMETERS ===\n";
        std::cout << "Pitch Bend Range: " << static_cast<int>(pitchBendRange) << " semitones\n";
        std::cout << "Mod Wheel Intensity: " << static_cast<int>(modWheelIntensity) << "\n";
        std::cout << "Mod Wheel Assignment:\n";
        std::cout << "  - Pitch Mod Depth: " << (modWheelAssignment.pitchModDepth ? "ON" : "OFF") << "\n";
        std::cout << "  - Amp Mod Depth: " << (modWheelAssignment.ampModDepth ? "ON" : "OFF") << "\n";
        std::cout << "  - EG Bias: " << (modWheelAssignment.egBias ? "ON" : "OFF") << "\n";
        std::cout << "MIDI Channel: " << static_cast<int>(midiChannel) << (midiChannel == 0 ? " (OMNI)" : "") << "\n";
        std::cout << "=========================" << std::endl;
        #endif

        #ifdef DEBUG_TEENSY
        Serial.println(F("=== GLOBAL PARAMETERS ==="));
        Serial.print(F("Pitch Bend Range: "));
        Serial.print(pitchBendRange);
        Serial.println(F(" semitones"));
        Serial.print(F("Mod Wheel Intensity: "));
        Serial.println(modWheelIntensity);
        Serial.println(F("Mod Wheel Assignment:"));
        Serial.print(F("  - Pitch Mod Depth: "));
        Serial.println(modWheelAssignment.pitchModDepth ? F("ON") : F("OFF"));
        Serial.print(F("  - Amp Mod Depth: "));
        Serial.println(modWheelAssignment.ampModDepth ? F("ON") : F("OFF"));
        Serial.print(F("  - EG Bias: "));
        Serial.println(modWheelAssignment.egBias ? F("ON") : F("OFF"));
        Serial.print(F("MIDI Channel: "));
        Serial.print(midiChannel);
        if (midiChannel == 0) {
            Serial.print(F(" (OMNI)")); 
        }
        Serial.println();
        Serial.println(F("========================="));
        #endif
    }
};

#endif // PARAMS_H