#ifndef WIDGET_TYPES_H
#define WIDGET_TYPES_H

#include <cstdint>

// Widget types for parameter display
enum class WidgetType : uint8_t {
    KNOB,                    // Rotary knob (most common)
    TOGGLE,                  // ON/OFF switch
    SELECTOR,                // Multi-position selector (ratio/fixed, curves, etc.)
    WAVEFORM_LFO,           // LFO waveform preview
    WAVEFORM_LEVEL_SCALING, // Level scaling curve preview
    WAVEFORM_OSC,           // Oscillator waveform preview
    ALGORITHM_DIAGRAM,      // Algorithm routing diagram (special for algorithm page)
    LIST,                   // Scrollable list (for bank/preset pages)
    TEXT_DISPLAY,           // Simple text/number display
    LARGE_VALUE             // Large centered value (for preset selection)
};

// Widget descriptor - describes a parameter and its visual representation
struct WidgetDescriptor {
    const char* label;      // Parameter label (e.g., "Speed", "Attack", "Coarse")
    WidgetType type;        // Widget type
    uint8_t position;       // Grid position (0-7: row0=[0,1,2,3], row1=[4,5,6,7])
    uint8_t spanCols;       // How many columns to span (1-4, default 1)
    uint8_t spanRows;       // How many rows to span (1-2, default 1)
    void* valuePtr;         // Pointer to the actual value (can be uint8_t*, bool*, etc.)
    int16_t minValue;       // Min value (for knobs/selectors)
    int16_t maxValue;       // Max value
    
    // Constructor for easier initialization
    WidgetDescriptor(const char* lbl, WidgetType t, uint8_t pos, void* val, 
                     int16_t min = 0, int16_t max = 99, uint8_t colSpan = 1, uint8_t rowSpan = 1)
        : label(lbl), type(t), position(pos), spanCols(colSpan), spanRows(rowSpan), 
          valuePtr(val), minValue(min), maxValue(max) {}
};

// Grid layout constants
constexpr uint8_t GRID_COLS = 4;
constexpr uint8_t GRID_ROWS = 2;
constexpr uint8_t GRID_TOTAL = GRID_COLS * GRID_ROWS;  // 8 positions

// Helper to get row/col from position
inline uint8_t getGridRow(uint8_t position) { return position / GRID_COLS; }
inline uint8_t getGridCol(uint8_t position) { return position % GRID_COLS; }

#endif // WIDGET_TYPES_H
