#define DEBUG_TEENSY

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "../core/synth.h"
#include "../core/config.h"
#include "../core/lut.h"
#include "../core/sysex.h"

#include "audio.h"

Synth synth;
SynthConfig config;
SysexHandler sysex;

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
        if (sysex.loadPreset(&config, 10)) {
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
}

void loop() {
    delay(1000);

    synth.noteOn(60, 80); // Middle C
    delay(1000);
    synth.noteOn(64, 80); // E
    delay(1000);
    synth.noteOn(67, 80); // G

    Serial.print("CPU: ");
    Serial.print(AudioProcessorUsage());
    Serial.println("%");

    delay(5000);

    synth.noteOff(60);
    synth.noteOff(64);
    synth.noteOff(67);

    delay(1000);
}