#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include <ILI9488_t3.h>
#include <SPI.h>

// ILI9488_t3 wrapper for MSP3521 (480x320) on Teensy 4.1 SPI1
class LcdDisplay {
public:
    // Teensy 4.1 pin mapping for MSP3521 on SPI1
    static constexpr uint8_t PIN_CS   = 38; // A14
    static constexpr uint8_t PIN_DC   = 36; // DC/RS
    static constexpr uint8_t PIN_RST  = 37; // RESET
    static constexpr uint8_t PIN_BL   = 2;  // OUT2 for LED/backlight
    static constexpr uint8_t PIN_MOSI = 26; // MOSI1
    static constexpr uint8_t PIN_MISO = 39; // A15 / MISO1
    static constexpr uint8_t PIN_SCK  = 27; // SCK1

    LcdDisplay() : tft(PIN_CS, PIN_DC, PIN_RST, PIN_MOSI, PIN_SCK, PIN_MISO), initialized(false) {}

    bool init() {
        // Backlight at full brightness
        pinMode(PIN_BL, OUTPUT);
        digitalWrite(PIN_BL, HIGH);

        // Initialize ILI9488 on SPI1
        tft.begin();
        tft.setRotation(3); // Landscape 480x320 (flipped)
        tft.fillScreen(ILI9488_BLACK);

        initialized = true;
        return true;
    }

    // Display color test pattern (TV-style color bars)
    void showTestScreen() {
        if (!initialized) return;

        tft.fillScreen(ILI9488_BLACK);
        
        // Top bar: white text
        tft.setTextColor(ILI9488_WHITE);
        tft.setTextSize(2);
        tft.setCursor(10, 10);
        tft.print(F("MSP3521 ILI9488 - 480x320"));
        
        // Color bars (8 vertical bars)
        const uint16_t barWidth = 60;
        const uint16_t barHeight = 200;
        const uint16_t startY = 60;
        
        tft.fillRect(0,   startY, barWidth, barHeight, ILI9488_WHITE);
        tft.fillRect(60,  startY, barWidth, barHeight, ILI9488_YELLOW);
        tft.fillRect(120, startY, barWidth, barHeight, ILI9488_CYAN);
        tft.fillRect(180, startY, barWidth, barHeight, ILI9488_GREEN);
        tft.fillRect(240, startY, barWidth, barHeight, ILI9488_MAGENTA);
        tft.fillRect(300, startY, barWidth, barHeight, ILI9488_RED);
        tft.fillRect(360, startY, barWidth, barHeight, ILI9488_BLUE);
        tft.fillRect(420, startY, barWidth, barHeight, ILI9488_BLACK);
        
        // Bottom gradient bars (grey scale)
        const uint16_t greyY = startY + barHeight + 10;
        const uint16_t greyH = 40;
        for (int i = 0; i < 8; i++) {
            uint8_t grey = 255 - (i * 32);
            uint16_t color = tft.color565(grey, grey, grey);
            tft.fillRect(i * barWidth, greyY, barWidth, greyH, color);
        }
        
        #ifdef DEBUG_TEENSY
        if (Serial) Serial.println(F("LCD: Test screen displayed"));
        #endif
    }

    // Get access to TFT object for custom drawing
    ILI9488_t3& getTft() {
        return tft;
    }
    
    // Check if initialized
    bool isInitialized() const {
        return initialized;
    }

private:
    ILI9488_t3 tft;
    bool initialized;
};

#endif // LCD_H
