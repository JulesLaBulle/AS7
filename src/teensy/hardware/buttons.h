#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include <array>

// Hardware pins for 74HC165 shift registers
constexpr uint8_t BUTTON_DATA_PIN = 34;   // Serial data output (Q7 of second shift register)
constexpr uint8_t BUTTON_CLK_PIN = 33;    // Clock (shared)
constexpr uint8_t BUTTON_LOAD_PIN = 32;   // SH/LD# (shared)

constexpr uint8_t NUM_BUTTONS = 16;       // Two 8-bit shift registers

// Button callback function type
// Parameters: buttonIndex (0-15), pressed (true on rising edge)
typedef void (*ButtonCallback)(uint8_t buttonIndex, bool pressed);

// Button handler for 16 buttons via two cascaded 74HC165 shift registers
class ButtonsHandler {
private:
    std::array<bool, NUM_BUTTONS> buttonStates = {false};  // Current button states
    std::array<bool, NUM_BUTTONS> prevStates = {false};    // Previous states for edge detection
    ButtonCallback callback = nullptr;
    
    // Read all 16 buttons from cascaded shift registers
    // Returns 16-bit value where each bit represents a button state
    uint16_t readShiftRegisters() {
        // Load parallel inputs into shift registers (SH/LD# = LOW)
        digitalWrite(BUTTON_LOAD_PIN, LOW);
        delayMicroseconds(5);  // tSU: setup time for 74HC165
        digitalWrite(BUTTON_LOAD_PIN, HIGH);
        delayMicroseconds(5);  // tH: hold time
        
        uint16_t buttonData = 0;
        
        // Shift out all 16 bits (8 from each register)
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            // Read bit from second shift register's output
            uint8_t bit = digitalRead(BUTTON_DATA_PIN);
            
            // Buttons pull to GND when pressed (active LOW)
            // Invert logic so pressed = 1
            buttonData |= ((bit == LOW ? 1 : 0) << i);
            
            // Pulse clock to shift next bit
            digitalWrite(BUTTON_CLK_PIN, HIGH);
            delayMicroseconds(5);  // tW: pulse width
            digitalWrite(BUTTON_CLK_PIN, LOW);
        }
        
        return buttonData;
    }

public:
    ButtonsHandler() = default;
    
    // Initialize shift register pins
    void init() {
        pinMode(BUTTON_DATA_PIN, INPUT);
        pinMode(BUTTON_CLK_PIN, OUTPUT);
        pinMode(BUTTON_LOAD_PIN, OUTPUT);
        
        // Initialize pins to idle state
        digitalWrite(BUTTON_CLK_PIN, LOW);
        digitalWrite(BUTTON_LOAD_PIN, HIGH);
        
        // Read initial state to prevent false triggers
        uint16_t initialState = readShiftRegisters();
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            buttonStates[i] = (initialState >> i) & 0x01;
            prevStates[i] = buttonStates[i];
        }
    }
    
    // Poll buttons and detect state changes
    // Call this regularly from main loop
    void read() {
        uint16_t currentData = readShiftRegisters();
        
        // Check each button for state changes
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            bool currentState = (currentData >> i) & 0x01;
            buttonStates[i] = currentState;
            
            // Detect rising edge (button press)
            if (currentState && !prevStates[i]) {
                #ifdef DEBUG_TEENSY
                Serial.print(F("Button pressed: "));
                Serial.println(i);
                #endif
                
                // Call callback if registered
                if (callback) {
                    callback(i, true);
                }
            }
            
            prevStates[i] = currentState;
        }
    }
    
    // Register callback function for button events
    void setCallback(ButtonCallback cb) {
        callback = cb;
    }
    
    // Get current state of a specific button
    bool isPressed(uint8_t buttonIndex) const {
        if (buttonIndex >= NUM_BUTTONS) return false;
        return buttonStates[buttonIndex];
    }
    
    // Get all button states as bitmask
    uint16_t getAllStates() const {
        uint16_t states = 0;
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            if (buttonStates[i]) {
                states |= (1 << i);
            }
        }
        return states;
    }
};

#endif // BUTTONS_H
