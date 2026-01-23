#ifndef USER_PRESETS_H
#define USER_PRESETS_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// Platform-specific includes for file I/O
#ifdef PLATFORM_TEENSY
    #include <SD.h>
#else
    #include <fstream>
    #include <filesystem>
#endif

#ifdef DEBUG_PC
#include <iostream>
#endif

#include "config.h"

// User preset handler for .as7 files
// Stores complete SynthConfig (all parameters including non-DX7 extensions)
class UserPresetsHandler {
private:
    // Directory for user presets
    static constexpr const char* USER_PRESETS_DIR = "/presets/user";
    
    // List of user preset names (without extension)
    std::vector<std::string> presetNames;
    
    // Track if preset list is loaded
    bool presetsLoaded = false;
    
    // Magic number for file validation (ASCII "AS7\0")
    static constexpr uint32_t MAGIC_NUMBER = 0x00375341;
    static constexpr uint8_t FILE_VERSION = 1;
    
    // File header structure
    struct FileHeader {
        uint32_t magic;          // Magic number for validation
        uint8_t version;         // File format version
        uint8_t reserved[3];     // Reserved for future use
        char presetName[32];     // Preset name (null-terminated)
    };
    
    // Extracts filename without extension
    std::string extractFilename(const std::string& path) {
        size_t lastSlash = path.find_last_of("/\\");
        size_t lastDot = path.find_last_of(".");
        
        if (lastSlash == std::string::npos) {
            lastSlash = 0;
        } else {
            lastSlash++; // Skip the slash
        }
        
        if (lastDot == std::string::npos || lastDot < lastSlash) {
            return path.substr(lastSlash);
        }
        
        return path.substr(lastSlash, lastDot - lastSlash);
    }
    
    // Serialize SynthConfig to binary format
    void serializeConfig(const SynthConfig* config, uint8_t* buffer, size_t& offset) {
        // VoiceConfig
        for (int i = 0; i < NUM_OPERATORS; i++) {
            const OperatorConfig& op = config->voiceConfig.operatorConfigs[i];
            
            // Operator on/off
            buffer[offset++] = op.on ? 1 : 0;
            
            // FrequencyConfig
            buffer[offset++] = op.frequency.fixedFrequency ? 1 : 0;
            buffer[offset++] = op.frequency.detune;
            buffer[offset++] = op.frequency.coarse;
            buffer[offset++] = op.frequency.fine;
            
            // EnvelopeConfig
            buffer[offset++] = op.envelope.outputLevel;
            buffer[offset++] = op.envelope.l1;
            buffer[offset++] = op.envelope.l2;
            buffer[offset++] = op.envelope.l3;
            buffer[offset++] = op.envelope.l4;
            buffer[offset++] = op.envelope.r1;
            buffer[offset++] = op.envelope.r2;
            buffer[offset++] = op.envelope.r3;
            buffer[offset++] = op.envelope.r4;
            buffer[offset++] = op.envelope.rateScaling;
            
            // Other operator params
            buffer[offset++] = op.velocitySensitivity;
            buffer[offset++] = op.ampModSens;
            buffer[offset++] = op.lvlSclBreakpoint;
            buffer[offset++] = op.lvlSclLeftDepth;
            buffer[offset++] = op.lvlSclRightDepth;
            buffer[offset++] = op.lvlSclLeftCurve;
            buffer[offset++] = op.lvlSclRightCurve;
            buffer[offset++] = op.OSCKeySync ? 1 : 0;
            buffer[offset++] = op.waveform;  // NEW: waveform parameter
        }
        
        // Algorithm (store index, not pointer)
        uint8_t algoIndex = 0;
        for (size_t i = 0; i < Algorithms::NUM_ALGORITHMS; i++) {
            if (config->voiceConfig.algorithm == Algorithms::ALL_ALGORITHMS[i]) {
                algoIndex = i;
                break;
            }
        }
        buffer[offset++] = algoIndex;
        
        // Feedback and transpose
        buffer[offset++] = config->voiceConfig.feedback;
        buffer[offset++] = config->voiceConfig.transpose;
        
        // LFOConfig
        buffer[offset++] = config->lfoConfig.waveform;
        buffer[offset++] = config->lfoConfig.speed;
        buffer[offset++] = config->lfoConfig.delay;
        buffer[offset++] = config->lfoConfig.pitchModDepth;
        buffer[offset++] = config->lfoConfig.ampModDepth;
        buffer[offset++] = config->lfoConfig.pitchModSens;
        buffer[offset++] = config->lfoConfig.LFOKeySync ? 1 : 0;
        
        // PitchEnvelopeConfig
        buffer[offset++] = config->pitchEnvelopeConfig.l1;
        buffer[offset++] = config->pitchEnvelopeConfig.l2;
        buffer[offset++] = config->pitchEnvelopeConfig.l3;
        buffer[offset++] = config->pitchEnvelopeConfig.l4;
        buffer[offset++] = config->pitchEnvelopeConfig.r1;
        buffer[offset++] = config->pitchEnvelopeConfig.r2;
        buffer[offset++] = config->pitchEnvelopeConfig.r3;
        buffer[offset++] = config->pitchEnvelopeConfig.r4;
        
        // Monophonic mode
        buffer[offset++] = config->monophonic ? 1 : 0;
    }
    
    // Deserialize binary format to SynthConfig
    bool deserializeConfig(SynthConfig* config, const uint8_t* buffer, size_t& offset, size_t bufferSize) {
        if (offset >= bufferSize) return false;
        
        // VoiceConfig
        for (int i = 0; i < NUM_OPERATORS; i++) {
            if (offset + 23 >= bufferSize) return false;  // 23 bytes per operator
            
            OperatorConfig& op = config->voiceConfig.operatorConfigs[i];
            
            op.on = buffer[offset++] != 0;
            
            // FrequencyConfig
            op.frequency.fixedFrequency = buffer[offset++] != 0;
            op.frequency.detune = buffer[offset++];
            op.frequency.coarse = buffer[offset++];
            op.frequency.fine = buffer[offset++];
            
            // EnvelopeConfig
            op.envelope.outputLevel = buffer[offset++];
            op.envelope.l1 = buffer[offset++];
            op.envelope.l2 = buffer[offset++];
            op.envelope.l3 = buffer[offset++];
            op.envelope.l4 = buffer[offset++];
            op.envelope.r1 = buffer[offset++];
            op.envelope.r2 = buffer[offset++];
            op.envelope.r3 = buffer[offset++];
            op.envelope.r4 = buffer[offset++];
            op.envelope.rateScaling = buffer[offset++];
            
            // Other operator params
            op.velocitySensitivity = buffer[offset++];
            op.ampModSens = buffer[offset++];
            op.lvlSclBreakpoint = buffer[offset++];
            op.lvlSclLeftDepth = buffer[offset++];
            op.lvlSclRightDepth = buffer[offset++];
            op.lvlSclLeftCurve = buffer[offset++];
            op.lvlSclRightCurve = buffer[offset++];
            op.OSCKeySync = buffer[offset++] != 0;
            op.waveform = buffer[offset++];
        }
        
        if (offset + 17 >= bufferSize) return false;
        
        // Algorithm
        uint8_t algoIndex = buffer[offset++];
        if (algoIndex < Algorithms::NUM_ALGORITHMS) {
            config->voiceConfig.algorithm = Algorithms::ALL_ALGORITHMS[algoIndex];
        }
        
        // Feedback and transpose
        config->voiceConfig.feedback = buffer[offset++];
        config->voiceConfig.transpose = buffer[offset++];
        
        // LFOConfig
        config->lfoConfig.waveform = buffer[offset++];
        config->lfoConfig.speed = buffer[offset++];
        config->lfoConfig.delay = buffer[offset++];
        config->lfoConfig.pitchModDepth = buffer[offset++];
        config->lfoConfig.ampModDepth = buffer[offset++];
        config->lfoConfig.pitchModSens = buffer[offset++];
        config->lfoConfig.LFOKeySync = buffer[offset++] != 0;
        
        // PitchEnvelopeConfig
        config->pitchEnvelopeConfig.l1 = buffer[offset++];
        config->pitchEnvelopeConfig.l2 = buffer[offset++];
        config->pitchEnvelopeConfig.l3 = buffer[offset++];
        config->pitchEnvelopeConfig.l4 = buffer[offset++];
        config->pitchEnvelopeConfig.r1 = buffer[offset++];
        config->pitchEnvelopeConfig.r2 = buffer[offset++];
        config->pitchEnvelopeConfig.r3 = buffer[offset++];
        config->pitchEnvelopeConfig.r4 = buffer[offset++];
        
        // Monophonic mode
        config->monophonic = buffer[offset++] != 0;
        
        return true;
    }
    
public:
    UserPresetsHandler() = default;
    
    // Save current config as user preset
    bool savePreset(const SynthConfig* config, const std::string& presetName) {
        if (presetName.empty()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Preset name cannot be empty" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Preset name cannot be empty"));
            #endif
            return false;
        }
        
        // Build filename
        std::string filename = std::string(USER_PRESETS_DIR) + "/" + presetName + ".as7";
        
        // Prepare header
        FileHeader header;
        header.magic = MAGIC_NUMBER;
        header.version = FILE_VERSION;
        header.reserved[0] = 0;
        header.reserved[1] = 0;
        header.reserved[2] = 0;
        strncpy(header.presetName, presetName.c_str(), sizeof(header.presetName) - 1);
        header.presetName[sizeof(header.presetName) - 1] = '\0';
        
        // Serialize config to buffer
        uint8_t configBuffer[256];  // Max config size
        size_t offset = 0;
        serializeConfig(config, configBuffer, offset);
        
        #ifdef PLATFORM_TEENSY
        // Teensy: Write to SD card
        
        // Create directory if it doesn't exist
        if (!SD.exists(USER_PRESETS_DIR)) {
            SD.mkdir(USER_PRESETS_DIR);
        }
        
        File file = SD.open(filename.c_str(), FILE_WRITE);
        if (!file) {
            #ifdef DEBUG_TEENSY
            Serial.print(F("Error: Could not create file "));
            Serial.println(filename.c_str());
            #endif
            return false;
        }
        
        // Write header
        file.write(reinterpret_cast<const uint8_t*>(&header), sizeof(FileHeader));
        
        // Write config data
        file.write(configBuffer, offset);
        
        file.close();
        
        #ifdef DEBUG_TEENSY
        Serial.print(F("Successfully saved user preset: "));
        Serial.println(presetName.c_str());
        #endif
        
        #else
        // PC: Write to filesystem
        
        // Create directory if it doesn't exist
        std::filesystem::create_directories(USER_PRESETS_DIR);
        
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not create file " << filename << std::endl;
            #endif
            return false;
        }
        
        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(FileHeader));
        
        // Write config data
        file.write(reinterpret_cast<const char*>(configBuffer), offset);
        
        file.close();
        
        #ifdef DEBUG_PC
        std::cout << "Successfully saved user preset: " << presetName << std::endl;
        #endif
        #endif
        
        return true;
    }
    
    // Load user presets directory (scan for .as7 files)
    bool loadUserBank() {
        presetNames.clear();
        presetsLoaded = false;
        
        #ifdef PLATFORM_TEENSY
        // Teensy: List directory
        if (!SD.exists(USER_PRESETS_DIR)) {
            #ifdef DEBUG_TEENSY
            Serial.println(F("User presets directory does not exist, creating..."));
            #endif
            SD.mkdir(USER_PRESETS_DIR);
            presetsLoaded = true;
            return true;
        }
        
        File dir = SD.open(USER_PRESETS_DIR);
        if (!dir) {
            #ifdef DEBUG_TEENSY
            Serial.print(F("Error: Could not open directory "));
            Serial.println(USER_PRESETS_DIR);
            #endif
            return false;
        }
        
        while (true) {
            File entry = dir.openNextFile();
            if (!entry) break;
            
            if (!entry.isDirectory()) {
                std::string filename = entry.name();
                // Check if .as7 extension
                if (filename.length() > 4 && 
                    filename.substr(filename.length() - 4) == ".as7") {
                    presetNames.push_back(extractFilename(filename));
                }
            }
            entry.close();
        }
        dir.close();
        
        #else
        // PC: List directory
        if (!std::filesystem::exists(USER_PRESETS_DIR)) {
            #ifdef DEBUG_PC
            std::cout << "User presets directory does not exist, creating..." << std::endl;
            #endif
            std::filesystem::create_directories(USER_PRESETS_DIR);
            presetsLoaded = true;
            return true;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(USER_PRESETS_DIR)) {
            if (entry.is_regular_file() && entry.path().extension() == ".as7") {
                presetNames.push_back(entry.path().stem().string());
            }
        }
        #endif
        
        presetsLoaded = true;
        
        #ifdef DEBUG_PC
        std::cout << "Found " << presetNames.size() << " user presets" << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Found "));
        Serial.print(presetNames.size());
        Serial.println(F(" user presets"));
        #endif
        
        return true;
    }
    
    // Load a specific user preset by index
    bool loadPreset(SynthConfig* config, uint8_t presetIndex) {
        if (!presetsLoaded) {
            #ifdef DEBUG_PC
            std::cerr << "Error: User bank not loaded" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: User bank not loaded"));
            #endif
            return false;
        }
        
        if (presetIndex >= presetNames.size()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Preset index out of range" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Preset index out of range"));
            #endif
            return false;
        }
        
        return loadPresetByName(config, presetNames[presetIndex]);
    }
    
    // Load a specific user preset by name
    bool loadPresetByName(SynthConfig* config, const std::string& presetName) {
        std::string filename = std::string(USER_PRESETS_DIR) + "/" + presetName + ".as7";
        
        #ifdef PLATFORM_TEENSY
        // Teensy: Read from SD card
        File file = SD.open(filename.c_str());
        if (!file) {
            #ifdef DEBUG_TEENSY
            Serial.print(F("Error: Could not open file "));
            Serial.println(filename.c_str());
            #endif
            return false;
        }
        
        size_t size = file.size();
        std::vector<uint8_t> buffer(size);
        size_t bytesRead = file.read(buffer.data(), size);
        file.close();
        
        if (bytesRead < size) {
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Could not read file"));
            #endif
            return false;
        }
        
        #else
        // PC: Read from filesystem
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not open file " << filename << std::endl;
            #endif
            return false;
        }
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not read file " << filename << std::endl;
            #endif
            return false;
        }
        #endif
        
        // Validate header
        if (buffer.size() < sizeof(FileHeader)) {
            #ifdef DEBUG_PC
            std::cerr << "Error: File too small" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: File too small"));
            #endif
            return false;
        }
        
        const FileHeader* header = reinterpret_cast<const FileHeader*>(buffer.data());
        
        if (header->magic != MAGIC_NUMBER) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Invalid file format (magic number mismatch)" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Invalid file format"));
            #endif
            return false;
        }
        
        if (header->version != FILE_VERSION) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Unsupported file version" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Unsupported file version"));
            #endif
            return false;
        }
        
        // Deserialize config
        size_t offset = sizeof(FileHeader);
        if (!deserializeConfig(config, buffer.data(), offset, buffer.size())) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Failed to deserialize config" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Failed to deserialize config"));
            #endif
            return false;
        }
        
        #ifdef DEBUG_PC
        std::cout << "Successfully loaded user preset: " << presetName << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Successfully loaded user preset: "));
        Serial.println(presetName.c_str());
        #endif
        
        return true;
    }
    
    // Get list of preset names
    const std::vector<std::string>& getPresetNames() const {
        return presetNames;
    }
    
    // Get number of user presets
    size_t getPresetCount() const {
        return presetNames.size();
    }
    
    // Check if bank is loaded
    bool isBankLoaded() const {
        return presetsLoaded;
    }
    
    // Delete a user preset by index and reload bank
    bool deletePreset(uint8_t presetIndex) {
        if (!presetsLoaded) {
            #ifdef DEBUG_PC
            std::cerr << "Error: User bank not loaded" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: User bank not loaded"));
            #endif
            return false;
        }
        
        if (presetIndex >= presetNames.size()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Invalid preset index" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Invalid preset index"));
            #endif
            return false;
        }
        
        // Build filename
        std::string filename = std::string(USER_PRESETS_DIR) + "/" + presetNames[presetIndex] + ".as7";
        
        #ifdef DEBUG_PC
        std::cout << "Deleting preset: " << presetNames[presetIndex] << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Deleting preset: "));
        Serial.println(presetNames[presetIndex].c_str());
        #endif
        
        // Delete file
        #ifdef PLATFORM_TEENSY
        if (!SD.remove(filename.c_str())) {
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Could not delete file"));
            #endif
            return false;
        }
        #else
        if (!std::filesystem::remove(filename)) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not delete file " << filename << std::endl;
            #endif
            return false;
        }
        #endif
        
        // Reload bank to refresh list
        loadUserBank();
        
        #ifdef DEBUG_PC
        std::cout << "Preset deleted successfully" << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.println(F("Preset deleted successfully"));
        #endif
        
        return true;
    }
    
    // Delete a user preset by name and reload bank
    bool deletePresetByName(const std::string& presetName) {
        if (!presetsLoaded) {
            #ifdef DEBUG_PC
            std::cerr << "Error: User bank not loaded" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: User bank not loaded"));
            #endif
            return false;
        }
        
        // Find preset index by name
        for (size_t i = 0; i < presetNames.size(); i++) {
            if (presetNames[i] == presetName) {
                return deletePreset(static_cast<uint8_t>(i));
            }
        }
        
        #ifdef DEBUG_PC
        std::cerr << "Error: Preset not found: " << presetName << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Error: Preset not found: "));
        Serial.println(presetName.c_str());
        #endif
        
        return false;
    }
    
    // Unload user bank (called when loading ROM bank)
    void unloadUserBank() {
        presetsLoaded = false;
        presetNames.clear();
    }
};

#endif // USER_PRESETS_H
