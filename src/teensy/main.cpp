#define DEBUG_TEENSY

#include <Arduino.h>
#include <SD.h>

#include "../core/synth.h"
#include "../core/config.h"
#include "../core/lut.h"
#include "../core/sysex.h"

// SD card chip select pin (Teensy 4.1 has built-in SD)
#define SD_CS_PIN BUILTIN_SDCARD

Synth synth;
SynthConfig config;
SysexHandler sysex;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for Serial or 3s timeout
    
    Serial.println(F("AS7 Initializing..."));
    
    LUT::init();
    synth.initParams();
    
    // Initialize SD card
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println(F("ERROR: SD card initialization failed!"));
        while (1); // Halt
    }
    
    // Load bank from SD
    if (sysex.loadBank("/presets/ROM1A_Master.syx")) {
        Serial.print(F("Bank loaded: "));
        Serial.println(sysex.getBankName().c_str());
        
        // Load preset
        if (sysex.loadPreset(&config, 0)) {
            synth.configure(&config);
        }
    }
    
    Serial.println(F("Ready!"));
    printSynthConfig(config);
}

void loop() {
    delay(100);
}