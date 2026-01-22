#ifndef RENDERER_H
#define RENDERER_H

#include <Arduino.h>
#include <ILI9488_t3.h>
#include <cstdint>
#include "widget_types.h"

// Display interface for UI pages
// Separates LOGIC (Page) from DISPLAY (Renderer)
// Current version: BASIC (text only)
// Future version: CUSTOM (knobs, sliders, curves, etc.)
//
// SCREEN LAYOUT (480x320 landscape):
//   ┌─────────────────────────────────┐
//   │ HEADER (60px)  - Title + info   │
//   ├─────────────────────────────────┤
//   │                                 │
//   │ CONTENT (230px) - Main content  │
//   │                                 │
//   ├─────────────────────────────────┤
//   │ FOOTER (30px)  - Encoder labels │
//   └─────────────────────────────────┘
//
// 8-COLUMN GRID: Screen divided into 8 columns (60px each) to align with 8 encoders
class Renderer {
private:
    ILI9488_t3* tft;
    
    // Layout constants
    static constexpr uint16_t SCREEN_WIDTH = 480;
    static constexpr uint16_t SCREEN_HEIGHT = 320;
    
    static constexpr uint16_t HEADER_HEIGHT = 60;
    static constexpr uint16_t CONTENT_HEIGHT = SCREEN_HEIGHT - HEADER_HEIGHT;  // 260px
    
    static constexpr uint16_t HEADER_Y = 0;
    static constexpr uint16_t CONTENT_Y = HEADER_HEIGHT;
    
    static constexpr uint8_t GRID_COLUMNS = 8;
    static constexpr uint16_t COLUMN_WIDTH = SCREEN_WIDTH / GRID_COLUMNS;  // 60px
    
    // Grid layout for widgets (2 rows x 4 columns)
    static constexpr uint16_t WIDGET_COLS = 4;
    static constexpr uint16_t WIDGET_ROWS = 2;
    static constexpr uint16_t WIDGET_WIDTH = SCREEN_WIDTH / WIDGET_COLS;   // 120px
    static constexpr uint16_t WIDGET_HEIGHT = CONTENT_HEIGHT / WIDGET_ROWS; // 130px
    
    // Retro monochrome LCD color palette (black background, yellow/green text)
    static constexpr uint16_t COLOR_BG = 0x0000;             // Pure black
    static constexpr uint16_t COLOR_HEADER_BG = 0x0000;      // Black (no distinction)
    static constexpr uint16_t COLOR_HEADER_TEXT = 0xFFE0;    // Bright yellow
    static constexpr uint16_t COLOR_TEXT = 0xE7E0;           // Yellow (slightly dimmed)
    static constexpr uint16_t COLOR_TEXT_DIM = 0x8C60;       // Dark yellow/olive
    static constexpr uint16_t COLOR_VALUE = 0xFFE0;          // Bright yellow for values
    static constexpr uint16_t COLOR_ACCENT = 0x07E0;         // Bright green for highlights
    
public:
    Renderer(ILI9488_t3* tftDisplay) : tft(tftDisplay) {}
    
    void clearScreen() {
        tft->fillScreen(COLOR_BG);
    }
    
    void clearContent() {
        tft->fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_HEIGHT, COLOR_BG);
    }
    
    // Clear only the value area of a parameter row (for lazy update)
    void clearParameterValue(uint8_t row) {
        if (row >= 8) return;
        uint16_t y = CONTENT_Y + 8 + (row * 32);
        // Clear right half of the row where values are displayed
        tft->fillRect(SCREEN_WIDTH / 2, y, SCREEN_WIDTH / 2, 24, COLOR_BG);
    }
    
    // Draw page header with title and optional subtitle
    void drawHeader(const char* title, const char* subtitle = nullptr) {
        tft->fillRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_HEADER_BG);
        
        // Draw header border
        tft->drawRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_TEXT_DIM);
        
        // Main title (centered horizontally and vertically)
        tft->setTextColor(COLOR_HEADER_TEXT);
        tft->setTextSize(3);
        int16_t titleHeight = 21;  // Approx height @ size 3
        int16_t x = (SCREEN_WIDTH - strlen(title) * 18) / 2;  // 18px per char @ size 3
        int16_t y;
        
        if (subtitle) {
            // If subtitle exists, position title higher
            y = HEADER_Y + (HEADER_HEIGHT - titleHeight - 24) / 2;  // Leave room for subtitle
        } else {
            // Center vertically in header
            y = HEADER_Y + (HEADER_HEIGHT - titleHeight) / 2;
        }
        
        tft->setCursor(x, y);
        tft->print(title);
        
        // Subtitle if present
        if (subtitle) {
            tft->setTextSize(2);
            x = (SCREEN_WIDTH - strlen(subtitle) * 12) / 2;  // 12px per char @ size 2
            tft->setCursor(x, y + 26);
            tft->print(subtitle);
        }
    }
    

    // Display parameter with label and numeric value (single line format)
    // @param clearValue: if true, clears old value before drawing (for lazy update)
    void drawParameter(uint8_t row, const char* label, int16_t value, bool clearValue = false) {
        if (row >= 8) return;
        
        uint16_t y = CONTENT_Y + 8 + (row * 32);  // 32px spacing for 8 rows in 260px
        
        // Label (left) - only drawn if not doing partial update
        if (!clearValue) {
            tft->setTextColor(COLOR_TEXT);
            tft->setTextSize(2);
            tft->setCursor(10, y);
            tft->print(label);
        }
        
        // Clear old value area if doing partial update
        if (clearValue) {
            clearParameterValue(row);
        }
        
        // Value (right-aligned)
        tft->setTextColor(COLOR_VALUE);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%d", value);
        int16_t valueWidth = strlen(buffer) * 12;  // 12px per char @ size 2
        tft->setTextSize(2);
        tft->setCursor(SCREEN_WIDTH - valueWidth - 10, y);
        tft->print(value);
    }
    
    // Display parameter with label and text value
    // @param clearValue: if true, clears old value before drawing (for lazy update)
    void drawParameter(uint8_t row, const char* label, const char* valueText, bool clearValue = false) {
        if (row >= 8) return;
        
        uint16_t y = CONTENT_Y + 8 + (row * 32);
        
        // Label (left) - only drawn if not doing partial update
        if (!clearValue) {
            tft->setTextColor(COLOR_TEXT);
            tft->setTextSize(2);
            tft->setCursor(10, y);
            tft->print(label);
        }
        
        // Clear old value area if doing partial update
        if (clearValue) {
            clearParameterValue(row);
        }
        
        // Right-align text value
        tft->setTextColor(COLOR_VALUE);
        int16_t valueWidth = strlen(valueText) * 12;  // 12px per char @ size 2
        tft->setTextSize(2);
        tft->setCursor(SCREEN_WIDTH - valueWidth - 10, y);
        tft->print(valueText);
    }
    
    // ==================
    // WIDGET RENDERING (Grid-based, 2x4 layout)
    // ==================
    
    // Draw complete widget (border + label + value) - used for full redraw
    void drawWidget(const WidgetDescriptor& widget) {
        if (widget.position >= GRID_TOTAL) return;
        
        uint8_t row = getGridRow(widget.position);
        uint8_t col = getGridCol(widget.position);
        
        uint16_t x = col * WIDGET_WIDTH;
        uint16_t y = CONTENT_Y + (row * WIDGET_HEIGHT);
        uint16_t w = WIDGET_WIDTH * widget.spanCols;
        uint16_t h = WIDGET_HEIGHT * widget.spanRows;
        
        // Draw border
        tft->drawRect(x, y, w, h, COLOR_TEXT_DIM);
        
        // Draw label (if present)
        if (widget.label && widget.label[0] != '\0') {
            tft->setTextColor(COLOR_TEXT);
            tft->setTextSize(2);
            tft->setCursor(x + 5, y + 5);
            tft->print(widget.label);
        }
        
        // Draw value
        drawWidgetValue(x, y + 30, w, h - 35, widget);
    }
    
    // Draw multiple widgets (full page layout)
    void drawWidgets(const WidgetDescriptor* widgets, uint8_t count) {
        for (uint8_t i = 0; i < count; i++) {
            drawWidget(widgets[i]);
        }
    }
    
    // Update ONLY the value area of a widget (efficient lazy update)
    // Clears and redraws only the variable content, not label/border
    void updateWidgetValue(const WidgetDescriptor& widget) {
        if (widget.position >= GRID_TOTAL) return;
        
        uint8_t row = getGridRow(widget.position);
        uint8_t col = getGridCol(widget.position);
        
        uint16_t x = col * WIDGET_WIDTH;
        uint16_t y = CONTENT_Y + (row * WIDGET_HEIGHT);
        uint16_t w = WIDGET_WIDTH * widget.spanCols;
        uint16_t h = WIDGET_HEIGHT * widget.spanRows;
        
        // Clear value area (below label at y+30)
        uint16_t valueY = y + 30;
        uint16_t valueH = h - 35;
        tft->fillRect(x + 2, valueY, w - 4, valueH, COLOR_BG);
        
        // Redraw value
        drawWidgetValue(x, valueY, w, valueH, widget);
    }
    
private:
    // === WIDGET VALUE RENDERING (private, called by drawWidget and updateWidgetValue) ===
    // Renders ONLY the value content, not border/label
    // x, y, w, h define the value area (not the full widget)
    
    void drawWidgetValue(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const WidgetDescriptor& widget) {
        switch (widget.type) {
            case WidgetType::KNOB:
            case WidgetType::SELECTOR:
                drawKnobValue(x, y, w, h, widget);
                break;
            case WidgetType::TOGGLE:
                drawToggleValue(x, y, w, h, widget);
                break;
            case WidgetType::WAVEFORM_LFO:
            case WidgetType::WAVEFORM_LEVEL_SCALING:
            case WidgetType::WAVEFORM_OSC:
                drawWaveformValue(x, y, w, h, widget);
                break;
            case WidgetType::ALGORITHM_DIAGRAM:
                drawAlgorithmDiagramValue(x, y, w, h, widget);
                break;
            default:
                break;
        }
    }
    
    void drawKnobValue(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const WidgetDescriptor& widget) {
        if (!widget.valuePtr) return;
        
        uint8_t value = *(static_cast<uint8_t*>(widget.valuePtr));
        
        tft->setTextColor(COLOR_VALUE);
        tft->setTextSize(4);
        
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d", value);
        
        // Center value
        int16_t textWidth = strlen(buffer) * 24;  // 24px per char @ size 4
        tft->setCursor(x + (w - textWidth) / 2, y + (h - 28) / 2);
        tft->print(value);
    }
    
    void drawToggleValue(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const WidgetDescriptor& widget) {
        if (!widget.valuePtr) return;
        
        bool value = *(static_cast<bool*>(widget.valuePtr));
        
        tft->setTextColor(value ? COLOR_ACCENT : COLOR_TEXT_DIM);
        tft->setTextSize(2);
        int16_t textWidth = value ? 24 : 36;  // "ON" vs "OFF"
        tft->setCursor(x + (w - textWidth) / 2, y + (h - 16) / 2);
        tft->print(value ? "ON" : "OFF");
    }
    
    void drawWaveformValue(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const WidgetDescriptor& widget) {
        // TODO: actual waveform preview
        (void)w; (void)h; (void)widget;
        tft->setTextColor(COLOR_TEXT_DIM);
        tft->setTextSize(1);
        tft->setCursor(x + 20, y + 20);
        tft->print("[WAVEFORM]");
    }
    
    void drawAlgorithmDiagramValue(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const WidgetDescriptor& widget) {
        // TODO: actual routing diagram
        (void)w; (void)h; (void)widget;
        tft->setTextColor(COLOR_TEXT_DIM);
        tft->setTextSize(2);
        tft->setCursor(x + 10, y + 10);
        tft->print("[ALGORITHM");
        tft->setCursor(x + 10, y + 40);
        tft->print(" DIAGRAM]");
    }
    
public:
    // ==================
    // LEGACY METHODS (kept for compatibility, can be phased out)
    // ==================
    
    // Display large centered value (useful for preset/bank selection)
    void drawLargeValue(int16_t value) {
        tft->setTextColor(COLOR_ACCENT);
        tft->setTextSize(6);
        
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%d", value);
        
        int16_t x = (SCREEN_WIDTH - strlen(buffer) * 36) / 2;  // 36px per char @ size 6
        int16_t y = CONTENT_Y + (CONTENT_HEIGHT / 2) - 30;
        
        tft->setCursor(x, y);
        tft->print(buffer);
    }
    
    // Display centered text (useful for preset/bank name)
    void drawCenteredText(const char* text, uint16_t offsetY = 0) {
        tft->setTextColor(COLOR_TEXT);
        tft->setTextSize(2);
        
        int16_t x = (SCREEN_WIDTH - strlen(text) * 12) / 2;
        int16_t y = CONTENT_Y + (CONTENT_HEIGHT / 2) + offsetY;
        
        tft->setCursor(x, y);
        tft->print(text);
    }
    
    // Display scrollable list (for Bank/Preset pages)
    // @param items: Array of strings to display
    // @param itemCount: Total number of items
    // @param selectedIndex: Index of highlighted item
    // @param startIndex: Index of first visible item (for scrolling)
    void drawList(const char* items[], uint8_t itemCount, uint8_t selectedIndex, uint8_t startIndex) {
        uint8_t visibleItems = 8;  // 8 items max visible (260px / 32px)
        
        for (uint8_t i = 0; i < visibleItems && (startIndex + i) < itemCount; i++) {
            uint8_t itemIndex = startIndex + i;
            uint16_t y = CONTENT_Y + 4 + (i * 32);
            
            // Highlight selected item
            if (itemIndex == selectedIndex) {
                tft->fillRect(0, y, SCREEN_WIDTH, 30, COLOR_HEADER_BG);
                tft->setTextColor(COLOR_ACCENT);
            } else {
                tft->setTextColor(COLOR_TEXT);
            }
            
            tft->setTextSize(2);
            tft->setCursor(10, y + 5);
            tft->print(items[itemIndex]);
        }
    }
    
    // Get X position of column center
    uint16_t getColumnCenterX(uint8_t column) const {
        if (column >= GRID_COLUMNS) return 0;
        return column * COLUMN_WIDTH + (COLUMN_WIDTH / 2);
    }
    
    uint16_t getColumnWidth() const {
        return COLUMN_WIDTH;
    }
    
    // Direct access to TFT for custom drawing
    ILI9488_t3* getTft() {
        return tft;
    }
};

#endif // RENDERER_H
