#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>
#include <array>

// Hardware pins for CD4051BE multiplexers
constexpr uint8_t ENC_MUX_A_PIN = 24;      // MUX select A (shared)
constexpr uint8_t ENC_MUX_B_PIN = 25;      // MUX select B (shared)
constexpr uint8_t ENC_MUX_C_PIN = 28;      // MUX select C (shared)

constexpr uint8_t ENC_PHASE_A_PIN = 29;    // MUX1 output (encoder phase A)
constexpr uint8_t ENC_PHASE_B_PIN = 30;    // MUX2 output (encoder phase B)
constexpr uint8_t ENC_BUTTON_PIN = 31;     // MUX3 output (encoder button)

constexpr uint8_t NUM_ENCODERS = 8;        // CD4051 has 8 channels

// Encoder rotation callback function type
// Parameters: encoderIndex (0-7), direction (1 = clockwise, -1 = counter-clockwise)
typedef void (*EncoderCallback)(uint8_t encoderIndex, int8_t direction);

// Encoder button callback function type
// Parameters: encoderIndex (0-7), pressed (true on rising edge)
typedef void (*EncoderButtonCallback)(uint8_t encoderIndex, bool pressed);

// Encoder handler for 8 rotary encoders with integrated buttons via CD4051BE multiplexers
class EncodersHandler {
private:
    std::array<uint8_t, NUM_ENCODERS> encoderStates = {0};    // Current quadrature states (0-3)
    std::array<int8_t, NUM_ENCODERS> encoderAccum = {0};      // Accumulated rotation for debouncing
    std::array<bool, NUM_ENCODERS> buttonStates = {false};    // Current button states
    std::array<bool, NUM_ENCODERS> prevButtonStates = {false}; // Previous button states
    
    EncoderCallback rotationCallback = nullptr;
    EncoderButtonCallback buttonCallback = nullptr;
    
    // Quadrature state transition table for rotation detection
    // [previous_state][current_state] -> direction
    // Returns: 1 = CW, -1 = CCW, 0 = no change or invalid
    // Table inverted to match correct CW/CCW direction
    static constexpr int8_t transitionTable[4][4] = {
        //  0   1   2   3  <- current state
        {  0, -1,  1,  0 }, // 0 <- previous state
        {  1,  0,  0, -1 }, // 1
        { -1,  0,  0,  1 }, // 2
        {  0,  1, -1,  0 }  // 3
    };
    
    // Select MUX channel (0-7)
    void selectMuxChannel(uint8_t channel) {
        if (channel > 7) return;
        
        digitalWrite(ENC_MUX_A_PIN, (channel & 0x01) ? HIGH : LOW);
        digitalWrite(ENC_MUX_B_PIN, (channel & 0x02) ? HIGH : LOW);
        digitalWrite(ENC_MUX_C_PIN, (channel & 0x04) ? HIGH : LOW);
        
        // Wait for MUX to settle (CD4051 propagation delay ~300ns typical)
        delayMicroseconds(1);
    }
    
    // Read current state of an encoder (phases A and B)
    // Returns: 2-bit state (0-3) where bit1=B, bit0=A
    uint8_t readEncoderState(uint8_t encoderIndex) {
        selectMuxChannel(encoderIndex);
        
        uint8_t phaseA = digitalRead(ENC_PHASE_A_PIN);
        uint8_t phaseB = digitalRead(ENC_PHASE_B_PIN);
        
        return (phaseB << 1) | phaseA;
    }
    
    // Read button state of an encoder
    bool readEncoderButton(uint8_t encoderIndex) {
        selectMuxChannel(encoderIndex);
        
        // Button pulls to GND when pressed (active LOW with pull-down)
        return (digitalRead(ENC_BUTTON_PIN) == HIGH);
    }

public:
    EncodersHandler() = default;
    
    // Initialize MUX pins and encoder inputs
    void init() {
        // MUX select pins (output)
        pinMode(ENC_MUX_A_PIN, OUTPUT);
        pinMode(ENC_MUX_B_PIN, OUTPUT);
        pinMode(ENC_MUX_C_PIN, OUTPUT);
        
        // MUX output pins (input)
        pinMode(ENC_PHASE_A_PIN, INPUT);
        pinMode(ENC_PHASE_B_PIN, INPUT);
        pinMode(ENC_BUTTON_PIN, INPUT);
        
        // Initialize MUX to channel 0
        selectMuxChannel(0);
        
        // Read initial states to prevent false triggers
        for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
            encoderStates[i] = readEncoderState(i);
            buttonStates[i] = readEncoderButton(i);
            prevButtonStates[i] = buttonStates[i];
        }
    }
    
    // Poll all encoders and detect rotation/button events
    // Call this regularly from main loop
    void read() {
        for (uint8_t i = 0; i < NUM_ENCODERS; i++) {
            // Read encoder rotation
            uint8_t prevState = encoderStates[i];
            uint8_t currentState = readEncoderState(i);
            
            if (currentState != prevState) {
                encoderStates[i] = currentState;
                
                // Lookup rotation direction from transition table
                int8_t direction = transitionTable[prevState][currentState];
                
                if (direction != 0) {
                    // Accumulate direction
                    encoderAccum[i] += direction;
                    
                    // Only report rotation when returning to rest state (state 0 = detent)
                    // This filters mechanical bounce and gives one event per click
                    if (currentState == 0 && encoderAccum[i] != 0) {
                        int8_t finalDirection = (encoderAccum[i] > 0) ? 1 : -1;
                        encoderAccum[i] = 0;  // Reset accumulator
                        
                        #ifdef DEBUG_TEENSY
                        Serial.print(F("Encoder "));
                        Serial.print(i);
                        Serial.print(F(" rotated "));
                        Serial.println(finalDirection > 0 ? F("CW") : F("CCW"));
                        #endif
                        
                        // Call rotation callback if registered
                        if (rotationCallback) {
                            rotationCallback(i, finalDirection);
                        }
                    }
                }
            }
            
            // Read encoder button
            bool currentButton = readEncoderButton(i);
            buttonStates[i] = currentButton;
            
            // Detect rising edge (button press)
            if (currentButton && !prevButtonStates[i]) {
                #ifdef DEBUG_TEENSY
                Serial.print(F("Encoder "));
                Serial.print(i);
                Serial.println(F(" button pressed"));
                #endif
                
                // Call button callback if registered
                if (buttonCallback) {
                    buttonCallback(i, true);
                }
            }
            
            prevButtonStates[i] = currentButton;
        }
    }
    
    // Register callback function for encoder rotation events
    void setRotationCallback(EncoderCallback cb) {
        rotationCallback = cb;
    }
    
    // Register callback function for encoder button events
    void setButtonCallback(EncoderButtonCallback cb) {
        buttonCallback = cb;
    }
    
    // Get current state of a specific encoder button
    bool isButtonPressed(uint8_t encoderIndex) const {
        if (encoderIndex >= NUM_ENCODERS) return false;
        return buttonStates[encoderIndex];
    }
    
    // Get current quadrature state of an encoder (0-3)
    uint8_t getEncoderState(uint8_t encoderIndex) const {
        if (encoderIndex >= NUM_ENCODERS) return 0;
        return encoderStates[encoderIndex];
    }
};

#endif // ENCODERS_H
