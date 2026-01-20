#define DEBUG_TEENSY

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "../core/synth.h"
#include "../core/config.h"
#include "../core/lut.h"
#include "../core/sysex.h"

#include "audio.h"
#include "hardware/midi.h"

Synth synth;
SynthConfig config;
SysexHandler sysex;
MidiHandler midi(&synth, 0); // Channel 0 (MIDI channel 1)

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for Serial or 3s timeout

    Serial.println(F("AS7 Program Starting..."));

    // ==================
    // Initialize SD card
    // ==================
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println(F("ERROR: SD card initialization failed!"));
        while (1); // Halt
    }
    Serial.println(F("SD card initialized successfully."));
    
    // ======================
    // Initialize synthesizer
    // ======================
    Serial.println(F("AS7 Initializing..."));
    LUT::init();
    synth.initParams();
    
    // Load bank from SD
    if (sysex.loadBank("/presets/ROM1A_Master.syx")) {
        Serial.print(F("Bank loaded: "));
        Serial.println(sysex.getBankName());
        
        // Load preset
        if (sysex.loadPreset(&config, 0)) {
            synth.configure(&config);
        }
    }

    printSynthConfig(config);

    // ================
    // Initialize audio
    // ================
    if (!Audio::init(&synth)) {
        Serial.println(F("ERROR: Audio initialization failed!"));
        while (1); // Halt
    }

    Serial.println(F("Ready!"));

    // ================
    // Initialize MIDI
    // ================
    midi.init();
    Serial.println(F("MIDI initialized on Serial1 (RX1/pin 1)"));
}

void loop() {
    // Process incoming MIDI messages
    midi.read();

    // Monitor CPU usage periodically
    static unsigned long lastCpuCheck = 0;
    unsigned long now = millis();
    if (now - lastCpuCheck >= 500) {
        lastCpuCheck = now;
        
        float cpuUsage = AudioProcessorUsage();
        float cpuUsageMax = AudioProcessorUsageMax();
        
        if (cpuUsage > 80.0f) {
            Serial.print(F("CPU HIGH - Usage: "));
            Serial.print(cpuUsage);
            Serial.print(F("% (Max: "));
            Serial.print(cpuUsageMax);
            Serial.println(F("%)"));
        }
        
        // Reset max for next measurement
        if (cpuUsageMax > 90.0f) {
            Serial.print(F("CPU CRITICAL - Max reached: "));
            Serial.print(cpuUsageMax);
            Serial.println(F("%"));
            AudioProcessorUsageMaxReset();
        }
    }
}