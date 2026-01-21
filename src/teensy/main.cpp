#define DEBUG_TEENSY

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "../core/synth.h"
#include "../core/config.h"
#include "../core/lut.h"
#include "../core/sysex.h"

#include "hardware/audio.h"
#include "hardware/midi.h"
#include "hardware/lcd.h"
#include "hardware/buttons.h"
#include "hardware/encoders.h"

Synth synth;
SynthConfig config;
SysexHandler sysex;
MidiHandler midi;
LcdDisplay lcd;
ButtonsHandler buttons;
EncodersHandler encoders;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for Serial or 3s timeout

    Serial.println(F("AS7 Program Starting..."));

    // ===============
    // Initialize LCD
    // ===============
    if (!lcd.init()) {
        Serial.println(F("ERROR: LCD initialization failed!"));
        while (1); // Halt
    } else {
        lcd.showTestScreen();
    }
    Serial.println(F("LCD initialized successfully."));

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
    Serial.println(F("AS7 Core Initializing..."));
    LUT::init();
    synth.initParams();
    
    // Load bank from SD
    if (sysex.loadBank("/presets/ROM1A_Master.syx")) {
        Serial.print(F("Bank loaded: "));
        Serial.println(sysex.getBankName());
        
        // Load preset
        if (sysex.loadPreset(&config, 10)) {
            synth.configure(&config);
            Serial.println(F("Core initialized successfully."));
        }
    }

    // ================
    // Initialize audio
    // ================
    if (!Audio::init(&synth)) {
        Serial.println(F("ERROR: Audio initialization failed!"));
        while (1); // Halt
    }
    Serial.println(F("Audio initialized successfully."));

    // ===============
    // Initialize MIDI
    // ===============
    midi.init(&synth);
    Serial.println(F("MIDI initialized on Serial1 (RX1/pin 1)"));
    
    // ==================
    // Initialize Buttons
    // ==================
    buttons.init();
    Serial.println(F("Buttons initialized (16 buttons via 74HC165)"));
    
    // ===================
    // Initialize Encoders
    // ===================
    encoders.init();
    Serial.println(F("Encoders initialized (8 encoders via CD4051)"));
    
    Serial.println(F("READY!"));
}

void loop() {
    midi.read();
    buttons.read();
    encoders.read();

    #ifdef DEBUG_TEENSY
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
    #endif
}