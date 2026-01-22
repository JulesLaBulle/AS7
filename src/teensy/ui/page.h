#ifndef PAGE_H
#define PAGE_H

#include <cstdint>

// Forward declarations
class Renderer;
class SynthConfig;
class Synth;

// Abstract base class for all UI pages
// Each page (Operator, LFO, Algorithm, etc.) inherits from this and implements specific behavior
//
// LIFECYCLE:
//   1. enter()   - Called when navigating to this page
//   2. update()  - Called repeatedly to refresh display
//   3. handleEncoder() / handleButton() - Handle user input
//   4. exit()    - Called when leaving this page
//
// UPDATE STRATEGY:
//   - fullRedraw = true  -> Redraw everything (enter, sub-page change)
//   - dirtyWidget >= 0   -> Update only that widget (encoder change)
//   - Both false/invalid -> No redraw
//
// RESPONSIBILITIES:
//   - Page: LOGIC (what values to modify, which widget changed)
//   - Renderer: DISPLAY (how to draw to screen)
//   - UIManager: NAVIGATION (which page to show, transitions)
class Page {
protected:
    SynthConfig* config;
    Synth* synth;
    Renderer* renderer;
    
    bool fullRedraw;        // True for full page redraw (enter, sub-page change)
    int8_t dirtyWidget;     // Index of widget to update (-1 = none, >=0 = specific widget)
    uint8_t subPage;        // Active sub-page index (0-N)
    
public:
    Page(SynthConfig* cfg, Synth* syn, Renderer* rend)
        : config(cfg), synth(syn), renderer(rend), fullRedraw(true), dirtyWidget(-1), subPage(0) {}
    
    virtual ~Page() = default;
    
    // Called when entering this page
    virtual void enter() {
        fullRedraw = true;
        dirtyWidget = -1;
        subPage = 0;
    }
    
    // Called when leaving this page
    virtual void exit() {}
    
    // Called repeatedly in loop() to refresh display (only redraws if needsRedraw == true)
    virtual void update() = 0;
    
    // Handle encoder rotation
    // @param encoderIndex: Which encoder (0-7)
    // @param direction: +1 (CW) or -1 (CCW)
    virtual void handleEncoder(uint8_t encoderIndex, int8_t direction) = 0;
    
    // Handle button press
    // @param buttonIndex: Which button (0-15) or encoder button (100-107 for encoders 0-7)
    // @return true if button was handled by page (prevents default navigation)
    virtual bool handleButton(uint8_t buttonIndex) = 0;
    
    // Change sub-page (prev/next with buttons 0-1)
    // @param direction: +1 (next) or -1 (prev)
    // @return true if changed, false if already at edge
    virtual bool changeSubPage(int8_t direction) {
        uint8_t maxSubPages = getSubPageCount();
        if (maxSubPages <= 1) return false;
        
        int16_t newSubPage = static_cast<int16_t>(subPage) + direction;
        
        if (newSubPage < 0) {
            newSubPage = 0;
            return false;
        } else if (newSubPage >= maxSubPages) {
            newSubPage = maxSubPages - 1;
            return false;
        }
        
        subPage = static_cast<uint8_t>(newSubPage);
        fullRedraw = true;
        dirtyWidget = -1;
        return true;
    }
    
    // Return number of sub-pages (override in derived classes if needed)
    virtual uint8_t getSubPageCount() const {
        return 1;  // Default: single page
    }
    
    uint8_t getCurrentSubPage() const {
        return subPage;
    }
};

#endif // PAGE_H
