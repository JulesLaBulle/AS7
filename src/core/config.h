#ifndef CONFIG_H
#define CONFIG_H

#include "constants.h"
#include <cstdint>

#ifdef DEBUG_PC
#include <iostream>
#endif

struct EnvelopeConfig {
    uint8_t outputLevel = 99; // Operator volume (0-99)

    uint8_t l1 = 99;    // Envelope levels (0-99)
    uint8_t l2 = 99;
    uint8_t l3 = 99;
    uint8_t l4 = 0;

    uint8_t r1 = 99;    // Envelope rates (0-99)
    uint8_t r2 = 0;
    uint8_t r3 = 0;
    uint8_t r4 = 99;

    uint8_t rateScaling = 0; // Rate scaling (0-7)
    
    EnvelopeConfig(uint8_t globalLevel = 99,
                   uint8_t level1 = 99, uint8_t level2 = 99, uint8_t level3 = 99, uint8_t level4 = 0,
                   uint8_t rate1 = 99,  uint8_t rate2 = 0,   uint8_t rate3 = 0,   uint8_t rate4 = 99,
                   uint8_t rateScale = 0)
        : outputLevel(globalLevel),
          l1(level1), l2(level2), l3(level3), l4(level4),
          r1(rate1), r2(rate2), r3(rate3), r4(rate4), 
          rateScaling(rateScale) {}
};

struct FrequencyConfig {
    bool fixedFrequency = false;  // false = ratio mode, true = fixed frequency mode
    
    uint8_t detune = 7;           // 0-14, center at 7 (no detune)
    uint8_t coarse = 0;           // 0-31 (0 = 0.5, 1 = 1, 2 = 2, etc.)
    uint8_t fine = 0;             // 0-99 (adds 0-99% of coarse value)
    
    FrequencyConfig(bool fixed = false, uint8_t d = 7, uint8_t c = 0, uint8_t f = 0)
        : fixedFrequency(fixed), detune(d), coarse(c), fine(f) {}
};

struct OperatorConfig {
    bool on = true;                     // Operator enabled/disabled
    
    FrequencyConfig frequency = FrequencyConfig();
    EnvelopeConfig envelope = EnvelopeConfig();

    uint8_t velocitySensitivity = 0;    // 0-7

    uint8_t ampModSens = 0;             // 0-3

    uint8_t lvlSclBreakpoint = 0;       // 0-99
    uint8_t lvlSclLeftDepth = 0;        // 0-99
    uint8_t lvlSclRightDepth = 0;
    uint8_t lvlSclLeftCurve = 0;        // 0-3  0=-LIN, -EXP, +EXP, +LIN
    uint8_t lvlSclRightCurve = 0;

    bool OSCKeySync = false;            // If true, oscillator restarts on new note
    uint8_t waveform = 0;               // 0=sine, 1=triangle, 2=saw down, 3=saw up, 4=square (default 0 for DX7 compatibility)

    OperatorConfig(bool OPon = true, FrequencyConfig freq = FrequencyConfig(), EnvelopeConfig env = EnvelopeConfig(), uint8_t vel = 0, uint8_t ams = 0,
                   uint8_t lscBP = 0, uint8_t lscLD = 0, uint8_t lscRD = 0, uint8_t lscLC = 0, uint8_t lscRC = 0, bool oscks = false, uint8_t wf = 0) 
        : on(OPon), frequency(freq), envelope(env), velocitySensitivity(vel), ampModSens(ams),
        lvlSclBreakpoint(lscBP), lvlSclLeftDepth(lscLD), lvlSclRightDepth(lscRD), lvlSclLeftCurve(lscLC), lvlSclRightCurve(lscRC), OSCKeySync(oscks), waveform(wf) {}
};

struct AlgorithmConfig {
    // Connection matrix: connections[modulator][carrier] = true if modulation exists
    // Fixed-size arrays for stack allocation and cache efficiency
    bool connections[NUM_OPERATORS][NUM_OPERATORS] = {{false}};
    
    // Precomputed data for fast processing
    uint8_t modulatorCount[NUM_OPERATORS] = {0};                    // Number of modulators per carrier
    uint8_t modulatorIndices[NUM_OPERATORS][NUM_OPERATORS] = {{0}}; // Indices of modulators for each carrier
    bool isCarrier[NUM_OPERATORS] = {false};                        // True if operator outputs audio
    
    bool hasFeedback = false;                // True if this algorithm uses feedback
    uint8_t feedbackOperator = 0;            // Which operator receives feedback (0-5)
    
    AlgorithmConfig() = default;
};

struct VoiceConfig {
    OperatorConfig operatorConfigs[NUM_OPERATORS];
    
    const AlgorithmConfig* algorithm = nullptr;
    
    uint8_t feedback = 0; // Feedback level (0-7)
    uint8_t transpose = 24; // Global transpose (in semitones)
    
    VoiceConfig() : algorithm(nullptr), feedback(0), transpose(24) {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operatorConfigs[i] = OperatorConfig();
        }
    }
    
    VoiceConfig(OperatorConfig opConfigs[NUM_OPERATORS], const AlgorithmConfig* alg, uint8_t fb = 0, uint8_t transp = 24) 
        : algorithm(alg), feedback(fb), transpose(transp) {
        for (size_t i = 0; i < NUM_OPERATORS; ++i) {
            operatorConfigs[i] = opConfigs[i];
        }
    }
};

struct LFOConfig {
    uint8_t waveform = 0;           // 0=triangle, 1=saw down, 2=saw up, 3=square, 4=sine, 5=sample&hold

    uint8_t speed = 0;              // 0-99
    uint8_t delay = 0;              // 0-99

    uint8_t pitchModDepth = 0;      // 0-99
    uint8_t ampModDepth = 0;        // 0-99

    uint8_t pitchModSens = 0;       // 0-7

    bool LFOKeySync = false;        // If true, LFO restarts on new note

    LFOConfig(uint8_t wf = 0, uint8_t sp = 0, uint8_t dl = 0, uint8_t pmd = 0, uint8_t amd = 0, uint8_t pms = 0, bool lfoks = false)
        : waveform(wf), speed(sp), delay(dl), pitchModDepth(pmd), ampModDepth(amd), pitchModSens(pms), LFOKeySync(lfoks) {}
};

struct PitchEnvelopeConfig {
    uint8_t l1 = 50;
    uint8_t l2 = 50;
    uint8_t l3 = 50;
    uint8_t l4 = 50;

    uint8_t r1 = 0;
    uint8_t r2 = 0;
    uint8_t r3 = 0;
    uint8_t r4 = 0;

    PitchEnvelopeConfig(uint8_t level1 = 50, uint8_t level2 = 50, uint8_t level3 = 50, uint8_t level4 = 50,
                        uint8_t rate1 = 0,  uint8_t rate2 = 0,   uint8_t rate3 = 0,   uint8_t rate4 = 0)
        : l1(level1), l2(level2), l3(level3), l4(level4),
          r1(rate1), r2(rate2), r3(rate3), r4(rate4) {}
};

struct SynthConfig {
    VoiceConfig voiceConfig = VoiceConfig();
    LFOConfig lfoConfig = LFOConfig();
    PitchEnvelopeConfig pitchEnvelopeConfig = PitchEnvelopeConfig();

    bool monophonic = false;
    
    SynthConfig(VoiceConfig vConfig = VoiceConfig(), LFOConfig lConfig = LFOConfig(), PitchEnvelopeConfig peConfig = PitchEnvelopeConfig(), bool mono = false)
        : voiceConfig(vConfig), lfoConfig(lConfig), pitchEnvelopeConfig(peConfig), monophonic(mono) {}
};

void printSynthConfig([[maybe_unused]] const SynthConfig& config) {
    #ifndef PLATFORM_TEENSY
    std::cout << "=== SYNTH CONFIGURATION ===\n\n";
    
    // 1. VOICE CONFIG
    std::cout << "=== VOICE CONFIG ===\n";
    std::cout << "Feedback: " << static_cast<int>(config.voiceConfig.feedback) << "\n";
    std::cout << "Transpose: " << static_cast<int>(config.voiceConfig.transpose) 
              << " (effective: " << static_cast<int>(config.voiceConfig.transpose) - 24 << " semitones)\n";
    std::cout << "Algorithm pointer: " << config.voiceConfig.algorithm << "\n";
    std::cout << "\n";
    
    // 2. OPERATOR CONFIGS
    for (size_t op = 0; op < NUM_OPERATORS; ++op) {
        const OperatorConfig& opConfig = config.voiceConfig.operatorConfigs[op];
        std::cout << "=== OPERATOR " << (op + 1) << " ===\n";
        std::cout << "  Enabled: " << (opConfig.on ? "Yes" : "No") << "\n";
        std::cout << "  Velocity Sensitivity: " << static_cast<int>(opConfig.velocitySensitivity) << "\n";
        std::cout << "  Amp Mod Sensitivity: " << static_cast<int>(opConfig.ampModSens) << "\n";
        std::cout << "  OSC Key Sync: " << (opConfig.OSCKeySync ? "Yes" : "No") << "\n";
        
        // Level Scaling
        std::cout << "  Level Scaling:\n";
        std::cout << "    Breakpoint: " << static_cast<int>(opConfig.lvlSclBreakpoint) << "\n";
        std::cout << "    Left Depth: " << static_cast<int>(opConfig.lvlSclLeftDepth) << "\n";
        std::cout << "    Right Depth: " << static_cast<int>(opConfig.lvlSclRightDepth) << "\n";
        std::cout << "    Left Curve: " << static_cast<int>(opConfig.lvlSclLeftCurve) << "\n";
        std::cout << "    Right Curve: " << static_cast<int>(opConfig.lvlSclRightCurve) << "\n";
        
        // Frequency Config
        const FrequencyConfig& freq = opConfig.frequency;
        std::cout << "  Frequency Config:\n";
        std::cout << "    Fixed Frequency: " << (freq.fixedFrequency ? "Yes" : "No") << "\n";
        std::cout << "    Detune: " << static_cast<int>(freq.detune) << "\n";
        std::cout << "    Coarse: " << static_cast<int>(freq.coarse) << "\n";
        std::cout << "    Fine: " << static_cast<int>(freq.fine) << "\n";
        
        // Envelope Config
        const EnvelopeConfig& env = opConfig.envelope;
        std::cout << "  Envelope Config:\n";
        std::cout << "    Output Level: " << static_cast<int>(env.outputLevel) << "\n";
        std::cout << "    Levels L1-L4: " 
                  << static_cast<int>(env.l1) << ", "
                  << static_cast<int>(env.l2) << ", "
                  << static_cast<int>(env.l3) << ", "
                  << static_cast<int>(env.l4) << "\n";
        std::cout << "    Rates R1-R4: " 
                  << static_cast<int>(env.r1) << ", "
                  << static_cast<int>(env.r2) << ", "
                  << static_cast<int>(env.r3) << ", "
                  << static_cast<int>(env.r4) << "\n";
        std::cout << "    Rate Scaling: " << static_cast<int>(env.rateScaling) << "\n";
        std::cout << "\n";
    }
    
    // 3. LFO CONFIG
    std::cout << "=== LFO CONFIG ===\n";
    std::cout << "Waveform: " << static_cast<int>(config.lfoConfig.waveform) 
              << " (0=tri, 1=saw↓, 2=saw↑, 3=sqr, 4=sin, 5=S&H)\n";
    std::cout << "Speed: " << static_cast<int>(config.lfoConfig.speed) << "\n";
    std::cout << "Delay: " << static_cast<int>(config.lfoConfig.delay) << "\n";
    std::cout << "Pitch Mod Depth: " << static_cast<int>(config.lfoConfig.pitchModDepth) << "\n";
    std::cout << "Amp Mod Depth: " << static_cast<int>(config.lfoConfig.ampModDepth) << "\n";
    std::cout << "Pitch Mod Sens: " << static_cast<int>(config.lfoConfig.pitchModSens) << "\n";
    std::cout << "LFO Key Sync: " << (config.lfoConfig.LFOKeySync ? "Yes" : "No") << "\n";
    std::cout << "\n";
    
    // 4. PITCH ENVELOPE CONFIG
    std::cout << "=== PITCH ENVELOPE CONFIG ===\n";
    std::cout << "Levels L1-L4: " 
              << static_cast<int>(config.pitchEnvelopeConfig.l1) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.l2) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.l3) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.l4) << "\n";
    std::cout << "Rates R1-R4: " 
              << static_cast<int>(config.pitchEnvelopeConfig.r1) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.r2) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.r3) << ", "
              << static_cast<int>(config.pitchEnvelopeConfig.r4) << "\n";
    std::cout << "\n";
    
    // 5. GLOBAL SETTINGS
    std::cout << "=== GLOBAL SETTINGS ===\n";
    std::cout << "Monophonic: " << (config.monophonic ? "Yes" : "No") << "\n";
    std::cout << "================================\n";
    #else
    Serial.println(F("=== SYNTH CONFIGURATION ==="));
    Serial.println();
    
    // 1. VOICE CONFIG
    Serial.println(F("=== VOICE CONFIG ==="));
    Serial.print(F("Feedback: "));
    Serial.println(config.voiceConfig.feedback);
    Serial.print(F("Transpose: "));
    Serial.print(config.voiceConfig.transpose);
    Serial.print(F(" (effective: "));
    Serial.print(static_cast<int>(config.voiceConfig.transpose) - 24);
    Serial.println(F(" semitones)"));
    Serial.print(F("Algorithm pointer: 0x"));
    Serial.println((unsigned long)config.voiceConfig.algorithm, HEX);
    Serial.println();
    
    // 2. OPERATOR CONFIGS
    for (size_t op = 0; op < NUM_OPERATORS; ++op) {
        const OperatorConfig& opConfig = config.voiceConfig.operatorConfigs[op];
        Serial.print(F("=== OPERATOR "));
        Serial.print(op + 1);
        Serial.println(F(" ==="));
        Serial.print(F("  Enabled: "));
        Serial.println(opConfig.on ? F("Yes") : F("No"));
        Serial.print(F("  Velocity Sensitivity: "));
        Serial.println(opConfig.velocitySensitivity);
        Serial.print(F("  Amp Mod Sensitivity: "));
        Serial.println(opConfig.ampModSens);
        Serial.print(F("  OSC Key Sync: "));
        Serial.println(opConfig.OSCKeySync ? F("Yes") : F("No"));
        
        // Level Scaling
        Serial.println(F("  Level Scaling:"));
        Serial.print(F("    Breakpoint: "));
        Serial.println(opConfig.lvlSclBreakpoint);
        Serial.print(F("    Left Depth: "));
        Serial.println(opConfig.lvlSclLeftDepth);
        Serial.print(F("    Right Depth: "));
        Serial.println(opConfig.lvlSclRightDepth);
        Serial.print(F("    Left Curve: "));
        Serial.println(opConfig.lvlSclLeftCurve);
        Serial.print(F("    Right Curve: "));
        Serial.println(opConfig.lvlSclRightCurve);
        
        // Frequency Config
        const FrequencyConfig& freq = opConfig.frequency;
        Serial.println(F("  Frequency Config:"));
        Serial.print(F("    Fixed Frequency: "));
        Serial.println(freq.fixedFrequency ? F("Yes") : F("No"));
        Serial.print(F("    Detune: "));
        Serial.println(freq.detune);
        Serial.print(F("    Coarse: "));
        Serial.println(freq.coarse);
        Serial.print(F("    Fine: "));
        Serial.println(freq.fine);
        
        // Envelope Config
        const EnvelopeConfig& env = opConfig.envelope;
        Serial.println(F("  Envelope Config:"));
        Serial.print(F("    Output Level: "));
        Serial.println(env.outputLevel);
        Serial.print(F("    Levels L1-L4: "));
        Serial.print(env.l1); Serial.print(F(", "));
        Serial.print(env.l2); Serial.print(F(", "));
        Serial.print(env.l3); Serial.print(F(", "));
        Serial.println(env.l4);
        Serial.print(F("    Rates R1-R4: "));
        Serial.print(env.r1); Serial.print(F(", "));
        Serial.print(env.r2); Serial.print(F(", "));
        Serial.print(env.r3); Serial.print(F(", "));
        Serial.println(env.r4);
        Serial.print(F("    Rate Scaling: "));
        Serial.println(env.rateScaling);
        Serial.println();
    }
    
    // 3. LFO CONFIG
    Serial.println(F("=== LFO CONFIG ==="));
    Serial.print(F("Waveform: "));
    Serial.print(config.lfoConfig.waveform);
    Serial.println(F(" (0=tri, 1=saw↓, 2=saw↑, 3=sqr, 4=sin, 5=S&H)"));
    Serial.print(F("Speed: "));
    Serial.println(config.lfoConfig.speed);
    Serial.print(F("Delay: "));
    Serial.println(config.lfoConfig.delay);
    Serial.print(F("Pitch Mod Depth: "));
    Serial.println(config.lfoConfig.pitchModDepth);
    Serial.print(F("Amp Mod Depth: "));
    Serial.println(config.lfoConfig.ampModDepth);
    Serial.print(F("Pitch Mod Sens: "));
    Serial.println(config.lfoConfig.pitchModSens);
    Serial.print(F("LFO Key Sync: "));
    Serial.println(config.lfoConfig.LFOKeySync ? F("Yes") : F("No"));
    Serial.println();
    
    // 4. PITCH ENVELOPE CONFIG
    Serial.println(F("=== PITCH ENVELOPE CONFIG ==="));
    Serial.print(F("Levels L1-L4: "));
    Serial.print(config.pitchEnvelopeConfig.l1); Serial.print(F(", "));
    Serial.print(config.pitchEnvelopeConfig.l2); Serial.print(F(", "));
    Serial.print(config.pitchEnvelopeConfig.l3); Serial.print(F(", "));
    Serial.println(config.pitchEnvelopeConfig.l4);
    Serial.print(F("Rates R1-R4: "));
    Serial.print(config.pitchEnvelopeConfig.r1); Serial.print(F(", "));
    Serial.print(config.pitchEnvelopeConfig.r2); Serial.print(F(", "));
    Serial.print(config.pitchEnvelopeConfig.r3); Serial.print(F(", "));
    Serial.println(config.pitchEnvelopeConfig.r4);
    Serial.println();
    
    // 5. GLOBAL SETTINGS
    Serial.println(F("=== GLOBAL SETTINGS ==="));
    Serial.print(F("Monophonic: "));
    Serial.println(config.monophonic ? F("Yes") : F("No"));
    Serial.println(F("================================"));
    #endif
}

#endif // CONFIG_H