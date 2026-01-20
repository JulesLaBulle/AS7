#ifndef MIDI_H
#define MIDI_H

#include <Arduino.h>
#include "../../core/synth.h"

// MIDI message decoder and handler
class MidiHandler {
private:
    Synth* synth;
    uint8_t channel;           // 0-15 (MIDI channels)
    uint8_t statusByte;        // Current running status
    uint8_t dataBytes[2];      // Temporary storage for data bytes
    uint8_t dataCount;         // Number of data bytes received
    
    // ================
    // Message handlers
    // ================
    
    void handleNoteOn(uint8_t note, uint8_t velocity) {
        synth->noteOn(note, velocity);
        #ifdef DEBUG_TEENSY
        Serial.print(F("NOTE ON  - Note: "));
        Serial.print(note);
        Serial.print(F(", Velocity: "));
        Serial.println(velocity);
        #endif
    }
    
    void handleNoteOff(uint8_t note, uint8_t velocity) {
        synth->noteOff(note);
        #ifdef DEBUG_TEENSY
        Serial.print(F("NOTE OFF - Note: "));
        Serial.println(note);
        #endif
    }
    
    void handlePitchBend(uint8_t lsb, uint8_t msb) {
        // TODO: Implement pitch bend modulation
        // int16_t bend = ((msb << 7) | lsb) - 8192; // -8192 to +8192
        #ifdef DEBUG_TEENSY
        int16_t bend = ((msb << 7) | lsb) - 8192;
        Serial.print(F("PITCH BEND - Value: "));
        Serial.println(bend);
        #endif
    }
    
    void handleModulation(uint8_t value) {
        // TODO: Implement modulation depth control
        #ifdef DEBUG_TEENSY
        Serial.print(F("MOD - Value: "));
        Serial.println(value);
        #endif
    }
    
    // Parse incoming byte as MIDI message
    void parseByte(uint8_t byte) {
        if (byte & 0x80) {
            // Status byte (MSB = 1)
            // Process any pending message before accepting new status
            if (dataCount == 2) {
                processMessage();
            }
            
            statusByte = byte;
            dataCount = 0;
            
            // Decode message type and channel
            uint8_t msgType = byte & 0xF0;
            uint8_t msgChannel = byte & 0x0F;
            
            // Ignore messages on other channels
            if (msgChannel != channel) {
                statusByte = 0; // Clear status to prevent running status
                return;
            }
            
            // Reset counter for new message (will accumulate data bytes)
            dataCount = 0;
        } else {
            // Data byte (MSB = 0)
            // Ignore if no valid status byte for our channel
            if (statusByte == 0 || (statusByte & 0x0F) != channel) {
                return;
            }
            
            if (dataCount < 2) {
                dataBytes[dataCount++] = byte;
            }
            
            // Process message when all data bytes received
            if (dataCount == 2) {
                processMessage();
                dataCount = 0;
            }
        }
    }
    
    // Execute action based on complete MIDI message
    void processMessage() {
        uint8_t msgType = statusByte & 0xF0;
        uint8_t msgChannel = statusByte & 0x0F;
        
        // Only process messages on target channel
        if (msgChannel != channel) return;
        
        uint8_t note = dataBytes[0];
        uint8_t value = dataBytes[1];
        
        switch (msgType) {
            case 0x90:  // Note On
                if (value > 0) {
                    handleNoteOn(note, value);
                } else {
                    handleNoteOff(note, 0);
                }
                break;
                
            case 0x80:  // Note Off
                handleNoteOff(note, value);
                break;
                
            case 0xE0:  // Pitch Bend
                handlePitchBend(dataBytes[0], dataBytes[1]);
                break;
                
            case 0xB0:  // Control Change
                if (dataBytes[0] == 0x01) {
                    // CC#1 = Modulation
                    handleModulation(dataBytes[1]);
                }
                break;
        }
    }

public:
    MidiHandler(Synth* synthPtr, uint8_t midiChannel = 0) 
        : synth(synthPtr), channel(midiChannel), statusByte(0), dataCount(0) {}
    
    // Initialize MIDI serial port
    void init() {
        Serial1.begin(31250); // MIDI standard baud rate
    }
    
    // Process all available MIDI bytes from serial buffer
    void read() {
        while (Serial1.available()) {
            parseByte(Serial1.read());
        }
    }
    
    // Set target MIDI channel (0-15)
    void setChannel(uint8_t ch) {
        if (ch <= 15) {
            channel = ch;
        }
    }
    
    // Get current target MIDI channel
    uint8_t getChannel() const {
        return channel;
    }
};

#endif // MIDI_H
