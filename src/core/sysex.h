#ifndef SYSEX_H
#define SYSEX_H

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// Platform-specific includes for file I/O
#ifdef PLATFORM_TEENSY
    #include <SD.h>
#else
    #include <fstream>
#endif

#ifdef DEBUG_PC
#include <iostream>
#endif

#include "core/config.h"
#include "core/connections.h"

class SysexHandler {
private:
    // Raw parameters for all 32 presets [preset][parameter]
    std::array<std::array<uint8_t, 155>, 32> bankParams;
    
    // Bank name (extracted from filename)
    std::string bankName;
    
    // Track if a bank is loaded
    bool bankLoaded = false;
    
    // List of available banks (without extension)
    std::vector<std::string> availableBanks;

    // Extracts filename from path
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

    // Unpacks 128 bytes of packed DX7 voice data into 155 parameters.
    void unpackVoice(const uint8_t* packedData, uint8_t* unpackedParams) {
        // Temporary buffer for unpacked parameters
        uint8_t tempParams[155] = {0};
        
        // Process each operator (6 operators, OP6 to OP1 in DX7 order)
        for (int op = 0; op < 6; ++op) {
            int base = op * 17;                 // 17 bytes per operator in packed format
            int paramBase = op * 21;            // 21 parameters per operator when unpacked

            // EG Rates and Levels (bytes 0-7)
            tempParams[paramBase + 0] = packedData[base + 0] & 0x7F;  // R1
            tempParams[paramBase + 1] = packedData[base + 1] & 0x7F;  // R2
            tempParams[paramBase + 2] = packedData[base + 2] & 0x7F;  // R3
            tempParams[paramBase + 3] = packedData[base + 3] & 0x7F;  // R4
            tempParams[paramBase + 4] = packedData[base + 4] & 0x7F;  // L1
            tempParams[paramBase + 5] = packedData[base + 5] & 0x7F;  // L2
            tempParams[paramBase + 6] = packedData[base + 6] & 0x7F;  // L3
            tempParams[paramBase + 7] = packedData[base + 7] & 0x7F;  // L4

            // Level Scaling (bytes 8-10)
            tempParams[paramBase + 8] = packedData[base + 8] & 0x7F;   // Break Point
            tempParams[paramBase + 9] = packedData[base + 9] & 0x7F;   // Left Depth
            tempParams[paramBase + 10] = packedData[base + 10] & 0x7F; // Right Depth

            // Left/Right Curve (byte 11)
            uint8_t leftrightcurves = packedData[base + 11] & 0x0F; // bits 4-7 are don't care
            tempParams[paramBase + 11] = leftrightcurves & 0x03;        // Left Curve (bits 1-0)
            tempParams[paramBase + 12] = (leftrightcurves >> 2) & 0x03; // Right Curve (bits 3-2)

            // Rate Scaling and Detune (byte 12)
            uint8_t detune_rs = packedData[base + 12] & 0x7F;
            tempParams[paramBase + 13] = detune_rs & 0x07;              // Rate Scaling (bits 2-0)
            tempParams[paramBase + 20] = (detune_rs >> 3) & 0x0F;       // Detune (bits 6-3)

            // Key Velocity Sensitivity and AMS (byte 13)
            uint8_t kvs_ams = packedData[base + 13] & 0x1F; // bits 5-7 are don't care
            tempParams[paramBase + 14] = kvs_ams & 0x03;               // AMS (bits 1-0)
            tempParams[paramBase + 15] = (kvs_ams >> 2) & 0x07;        // KVS (bits 4-2)

            // Output Level (byte 14)
            tempParams[paramBase + 16] = packedData[base + 14] & 0x7F; // Output Level

            // Oscillator Mode and Coarse (byte 15)
            uint8_t fcoarse_mode = packedData[base + 15] & 0x3F; // bits 6-7 are don't care
            tempParams[paramBase + 17] = fcoarse_mode & 0x01;               // Mode (bit 0)
            tempParams[paramBase + 18] = (fcoarse_mode >> 1) & 0x1F;        // Coarse (bits 5-1)

            // Fine Frequency (byte 16)
            tempParams[paramBase + 19] = packedData[base + 16] & 0x7F; // Fine
        }

        // Global parameters (starting at byte 102)
        int globalBase = 102;

        // Pitch EG (parameters 126-133)
        tempParams[126] = packedData[globalBase + 0] & 0x7F; // PR1
        tempParams[127] = packedData[globalBase + 1] & 0x7F; // PR2
        tempParams[128] = packedData[globalBase + 2] & 0x7F; // PR3
        tempParams[129] = packedData[globalBase + 3] & 0x7F; // PR4
        tempParams[130] = packedData[globalBase + 4] & 0x7F; // PL1
        tempParams[131] = packedData[globalBase + 5] & 0x7F; // PL2
        tempParams[132] = packedData[globalBase + 6] & 0x7F; // PL3
        tempParams[133] = packedData[globalBase + 7] & 0x7F; // PL4

        // Algorithm (parameter 134)
        tempParams[134] = packedData[globalBase + 8] & 0x1F; // Algorithm (0-31)

        // Feedback and Oscillator Key Sync (byte 111)
        uint8_t oks_fb = packedData[globalBase + 9] & 0x0F; // bits 4-7 are don't care
        tempParams[135] = oks_fb & 0x07;                     // Feedback (bits 2-0)
        tempParams[136] = (oks_fb >> 3) & 0x01;              // OSC Key Sync (bit 3)

        // LFO Parameters (bytes 112-115)
        tempParams[137] = packedData[globalBase + 10] & 0x7F; // LFO Speed
        tempParams[138] = packedData[globalBase + 11] & 0x7F; // LFO Delay
        tempParams[139] = packedData[globalBase + 12] & 0x7F; // LFO Pitch Mod Depth
        tempParams[140] = packedData[globalBase + 13] & 0x7F; // LFO Amp Mod Depth

        // LFO Waveform, Sync, and Pitch Mod Sensitivity (byte 116)
        uint8_t lpms_lfw_lks = packedData[globalBase + 14] & 0x7F;
        tempParams[141] = lpms_lfw_lks & 0x01;               // LFO Sync (bit 0)
        tempParams[142] = (lpms_lfw_lks >> 1) & 0x07;        // LFO Waveform (bits 3-1)
        tempParams[143] = (lpms_lfw_lks >> 4) & 0x07;        // LFO Pitch Mod Sensitivity (bits 6-4)

        // Transpose (parameter 144)
        tempParams[144] = packedData[globalBase + 15] & 0x7F; // Transpose

        // Voice Name (parameters 145-154, ASCII characters)
        for (int i = 0; i < 10; ++i) {
            tempParams[145 + i] = packedData[globalBase + 16 + i] & 0x7F;
        }

        // Copy to output array
        std::memcpy(unpackedParams, tempParams, 155);
    }
    
    // Converts unpacked parameters for a single voice to SynthConfig structure.
    void paramsToSynthConfig(const uint8_t* params, SynthConfig* config) {        
        // Create temporary config arrays
        EnvelopeConfig envConfigs[6];
        FrequencyConfig freqConfigs[6];
        OperatorConfig opConfigs[6];
        
        // Process each operator (DX7 order: OP6 to OP1, convert to our order: OP1 to OP6)
        for (int dx7Op = 0; dx7Op < 6; ++dx7Op) {
            int ourOp = 5 - dx7Op; // Reverse order: ourOp 0 = DX7 OP6, ourOp 5 = DX7 OP1
            int paramBase = dx7Op * 21;
            
            // Check parameter bounds before accessing
            if (paramBase + 20 >= 155) {
                #ifdef DEBUG_PC
                std::cerr << "Error: Parameter index out of bounds for operator " << dx7Op << std::endl;
                #endif
                #ifdef DEBUG_TEENSY
                Serial.print(F("Error: Parameter index out of bounds for operator "));
                Serial.println(dx7Op);
                #endif
                return;
            }
            
            // Build EnvelopeConfig
            envConfigs[ourOp] = EnvelopeConfig(
                params[paramBase + 16],                     // Output Level
                params[paramBase + 4], params[paramBase + 5], // L1, L2
                params[paramBase + 6], params[paramBase + 7], // L3, L4
                params[paramBase + 0], params[paramBase + 1], // R1, R2
                params[paramBase + 2], params[paramBase + 3], // R3, R4
                params[paramBase + 13]                        // Rate Scaling
            );
            
            // Build FrequencyConfig
            freqConfigs[ourOp] = FrequencyConfig(
                params[paramBase + 17] == 1,               // Fixed frequency mode
                params[paramBase + 20],                    // Detune (0-14, 7=center)
                params[paramBase + 18],                    // Coarse
                params[paramBase + 19]                     // Fine
            );
            
            // Build OperatorConfig
            opConfigs[ourOp] = OperatorConfig(
                true,                                      // Operator enabled
                freqConfigs[ourOp],                        // Frequency config
                envConfigs[ourOp],                         // Envelope config
                params[paramBase + 15],                    // Velocity Sensitivity
                params[paramBase + 14],                    // Amp Mod Sensitivity
                params[paramBase + 8],                     // Level Scaling Breakpoint
                params[paramBase + 9],                     // Level Scaling Left Depth
                params[paramBase + 10],                    // Level Scaling Right Depth
                params[paramBase + 11],                    // Level Scaling Left Curve
                params[paramBase + 12],                    // Level Scaling Right Curve
                params[136] == 1                           // OSC Key Sync
            );
        }
        
        // Set VoiceConfig
        VoiceConfig voiceConfig;
        
        // Check algorithm index bounds
        if (params[134] >= 32) {
            #ifdef DEBUG_PC
            std::cerr << "Warning: Algorithm index " << static_cast<int>(params[134]) 
                      << " out of range, using 0" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.print(F("Warning: Algorithm index "));
            Serial.print(params[134]);
            Serial.println(F(" out of range, using 0"));
            #endif
            voiceConfig.algorithm = Algorithms::ALL_ALGORITHMS[0];
        } else {
            voiceConfig.algorithm = Algorithms::ALL_ALGORITHMS[params[134]];
        }
        
        voiceConfig.feedback = params[135];
        voiceConfig.transpose = params[144];
        
        // Copy operator configurations
        for (int i = 0; i < 6; ++i) {
            voiceConfig.operatorConfigs[i] = opConfigs[i];
        }
        
        // Set LFOConfig
        LFOConfig lfoConfig(
            params[142],                                   // Waveform
            params[137],                                   // Speed
            params[138],                                   // Delay
            params[139],                                   // Pitch Mod Depth
            params[140],                                   // Amp Mod Depth
            params[143],                                   // Pitch Mod Sens
            params[141] == 1                               // LFO Key Sync
        );
        
        // Set PitchEnvelopeConfig
        PitchEnvelopeConfig pitchEnvConfig;
        pitchEnvConfig.r1 = params[126];
        pitchEnvConfig.r2 = params[127];
        pitchEnvConfig.r3 = params[128];
        pitchEnvConfig.r4 = params[129];
        pitchEnvConfig.l1 = params[130];
        pitchEnvConfig.l2 = params[131];
        pitchEnvConfig.l3 = params[132];
        pitchEnvConfig.l4 = params[133];
        
        // Build final SynthConfig
        *config = SynthConfig(voiceConfig, lfoConfig, pitchEnvConfig, false);
    }

public:
    SysexHandler() = default;
    
    // Load a DX7 bank file (32 presets).
    bool loadBank(const std::string& filename) {
        bankLoaded = false;

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
        
        // Check file size
        if (size != 4104) {
            #ifdef DEBUG_TEENSY
            Serial.print(F("Warning: File size is "));
            Serial.print(size);
            Serial.println(F(" bytes (expected 4104 for 32-voice DX7 dump)"));
            #endif
        }
        
        // Read into vector
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
        bankLoaded = false;

        // Open file
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not open file " << filename << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.print(F("Error: Could not open file "));
            Serial.println(filename.c_str());
            #endif
            return false;
        }
        
        // Get file size
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read entire file
        std::vector<uint8_t> buffer(static_cast<size_t>(size));
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Could not read file " << filename << std::endl;
            #endif
            return false;
        }
        #endif
        
        // Extract bank name from filename (same for both platforms)
        bankName = extractFilename(filename);
        
        // Check file size
        if (buffer.size() != 4104) {
            #ifdef DEBUG_PC
            std::cerr << "Warning: File size is " << buffer.size() 
                      << " bytes (expected 4104 for 32-voice DX7 dump)" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.print(F("Warning: File size is "));
            Serial.print(buffer.size());
            Serial.println(F(" bytes (expected 4104 for 32-voice DX7 dump)"));
            #endif
        }
        
        // Extract 32 voices (each 128 bytes of packed data)
        for (size_t voice = 0; voice < 32; ++voice) {
            size_t voiceOffset = 6 + (voice * 128);
            
            if (buffer.size() < voiceOffset + 128) {
                #ifdef DEBUG_PC
                std::cerr << "Error: File too small for 32 voices" << std::endl;
                #endif
                #ifdef DEBUG_TEENSY
                Serial.println(F("Error: File too small for 32 voices"));
                #endif
                bankLoaded = false;
                return false;
            }
            
            const uint8_t* packedVoice = buffer.data() + voiceOffset;
            unpackVoice(packedVoice, bankParams[voice].data());
        }
        
        bankLoaded = true;
        #ifdef DEBUG_PC
        std::cout << "Successfully loaded DX7 bank: " << bankName 
                  << " (" << buffer.size() << " bytes)" << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Successfully loaded DX7 bank: "));
        Serial.print(bankName.c_str());
        Serial.print(F(" ("));
        Serial.print(buffer.size());
        Serial.println(F(" bytes)"));
        #endif
        return true;
    }
    
    // Load a specific preset into a SynthConfig structure.
    bool loadPreset(SynthConfig* config, uint8_t presetIndex) {
        if (!bankLoaded) {
            #ifdef DEBUG_PC
            std::cerr << "Error: No bank loaded" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: No bank loaded"));
            #endif
            return false;
        }

        if (presetIndex >= 32) {
            #ifdef DEBUG_PC
            std::cerr << "Error: Preset index must be 0-31" << std::endl;
            #endif
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Preset index must be 0-31"));
            #endif
            return false;
        }

        const uint8_t* params = bankParams[presetIndex].data();

        paramsToSynthConfig(params, config);

        return true;
    }
    
    // Get the name of a preset.
    #ifdef PLATFORM_TEENSY
    // On Teensy: return pointer to preset name (zero-copy, no allocation)
    // The returned pointer points into bankParams, valid until next loadBank()
    const char* getPresetName(uint8_t presetIndex) const {
        static char invalidName[] = "Invalid";
        if (!bankLoaded || presetIndex >= 32) {
            return invalidName;
        }
        // Return pointer directly into bankParams (parameters 145-154 are the name)
        return reinterpret_cast<const char*>(&bankParams[presetIndex][145]);
    }
    #else
    // On PC: return std::string (simpler for debugging)
    std::string getPresetName(uint8_t presetIndex) const {
        if (!bankLoaded || presetIndex >= 32) {
            return "Invalid";
        }
        
        char name[11] = {0}; // 10 chars + null terminator
        for (int i = 0; i < 10; ++i) {
            name[i] = bankParams[presetIndex][145 + i];
        }
        return std::string(name);
    }
    #endif
    
    // Get all preset names at once (for menu display)
    #ifdef PLATFORM_TEENSY
    // On Teensy: fill provided buffer with pointers (zero-copy)
    // Each pointer points into bankParams, valid until next loadBank()
    void getAllPresetsNames(const char* names[32]) const {
        for (uint8_t i = 0; i < 32; ++i) {
            if (bankLoaded) {
                names[i] = reinterpret_cast<const char*>(&bankParams[i][145]);
            } else {
                static char empty[] = "";
                names[i] = empty;
            }
        }
    }
    #else
    // On PC: return array of std::string
    std::array<std::string, 32> getAllPresetsNames() const {
        std::array<std::string, 32> names;
        for (uint8_t i = 0; i < 32; ++i) {
            names[i] = getPresetName(i);
        }
        return names;
    }
    #endif
    
    // Get the loaded bank name.
    #ifdef PLATFORM_TEENSY
    // On Teensy: return pointer to bank name (zero-copy)
    const char* getBankName() const {
        return bankName.c_str();
    }
    #else
    // On PC: return std::string
    const std::string& getBankName() const {
        return bankName;
    }
    #endif
    
    // Check if a bank is loaded.
    bool isBankLoaded() const {
        return bankLoaded;
    }
    
    // Unload current bank (called when loading USER presets)
    void unloadBank() {
        bankLoaded = false;
        bankName.clear();
    }
    
    // Get raw parameters for a specific preset.
    const std::array<uint8_t, 155>& getRawPreset(uint8_t presetIndex) const {
        static const std::array<uint8_t, 155> empty = {};
        if (!bankLoaded || presetIndex >= 32) {
            return empty;
        }
        return bankParams[presetIndex];
    }
    
    // List all available .syx banks in /presets directory
    bool listBanks() {
        availableBanks.clear();
        
        #ifdef PLATFORM_TEENSY
        // Teensy: List SD card directory
        File dir = SD.open("/presets");
        if (!dir) {
            #ifdef DEBUG_TEENSY
            Serial.println(F("Error: Could not open /presets directory"));
            #endif
            return false;
        }
        
        File entry;
        while ((entry = dir.openNextFile())) {
            if (!entry.isDirectory()) {
                const char* name = entry.name();
                size_t len = strlen(name);
                
                // Check if ends with .syx
                if (len > 4 && strcmp(name + len - 4, ".syx") == 0) {
                    // Extract filename without extension
                    std::string filename(name, len - 4);
                    availableBanks.push_back(filename);
                }
            }
            entry.close();
        }
        dir.close();
        
        #else
        // PC: List filesystem directory
        if (!std::filesystem::exists("/presets")) {
            #ifdef DEBUG_PC
            std::cerr << "Error: /presets directory does not exist" << std::endl;
            #endif
            return false;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator("/presets")) {
            if (entry.is_regular_file() && entry.path().extension() == ".syx") {
                availableBanks.push_back(entry.path().stem().string());
            }
        }
        #endif
        
        #ifdef DEBUG_PC
        std::cout << "Found " << availableBanks.size() << " .syx banks" << std::endl;
        #endif
        #ifdef DEBUG_TEENSY
        Serial.print(F("Found "));
        Serial.print(availableBanks.size());
        Serial.println(F(" .syx banks"));
        #endif
        
        return true;
    }
    
    // Get list of available banks
    const std::vector<std::string>& getBanksList() const {
        return availableBanks;
    }
    
    // Get number of available banks
    size_t getBanksCount() const {
        return availableBanks.size();
    }
};

#endif // SYSEX_H