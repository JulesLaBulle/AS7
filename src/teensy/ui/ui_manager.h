#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <array>
#include "page.h"
#include "renderer.h"
#include "pages/page_algorithm.h"
#include "pages/page_lfo.h"
#include "pages/page_pitch_env.h"
#include "../hardware/lcd.h"
#include "../../core/config.h"
#include "../../core/synth.h"

// UI Manager - Orchestrates the user interface
// Handles page navigation, routes hardware events to active page, manages transitions
//
// BUTTON MAPPING (16 total):
//   Button 0  : PREV SUB-PAGE
//   Button 1  : NEXT SUB-PAGE
//   Button 2  : OPERATOR 1
//   Button 3  : OPERATOR 2
//   Button 4  : OPERATOR 3
//   Button 5  : OPERATOR 4
//   Button 6  : OPERATOR 5
//   Button 7  : OPERATOR 6
//   Button 8  : ALGORITHM
//   Button 9  : LFO
//   Button 10 : PITCH ENVELOPE
//   Button 11 : BANK
//   Button 12 : PRESET
//   Button 13 : SAVE
//   Button 14 : PARAMETERS
//
// ENCODERS (8 total): Handled by active page according to context
// ENCODER BUTTONS: Passed to handleButton() with offset (100-107 for encoders 0-7)
class UIManager {
public:
    // Offset for encoder buttons to avoid conflict with regular buttons
    static constexpr uint8_t ENCODER_BUTTON_OFFSET = 100;
    
    enum class PageType : uint8_t {
        OPERATOR_1 = 0,
        OPERATOR_2,
        OPERATOR_3,
        OPERATOR_4,
        OPERATOR_5,
        OPERATOR_6,
        ALGORITHM,
        LFO,
        PITCH_ENV,
        BANK,
        PRESET,
        SAVE,
        PARAMS,
        
        COUNT  // Total number of pages
    };
    
private:
    LcdDisplay* lcd;
    SynthConfig* config;
    Synth* synth;
    
    Renderer* renderer;
    
    // Page storage (array of pointers, filled during initialization)
    std::array<Page*, static_cast<size_t>(PageType::COUNT)> pages = {nullptr};
    
    PageType currentPageType;
    Page* currentPage;
    
    unsigned long lastUpdateTime;
    static constexpr unsigned long UPDATE_INTERVAL_MS = 50;  // 20 FPS
    
public:
    UIManager(LcdDisplay* lcdDisplay, SynthConfig* cfg, Synth* syn)
        : lcd(lcdDisplay), config(cfg), synth(syn), 
          currentPageType(PageType::ALGORITHM), currentPage(nullptr),
          lastUpdateTime(0) {
        
        renderer = new Renderer(&lcd->getTft());
        
        #ifdef DEBUG_TEENSY
        Serial.println(F("UIManager: Initialized"));
        #endif
    }
    
    ~UIManager() {
        for (auto& page : pages) {
            if (page) delete page;
        }
        if (renderer) delete renderer;
    }
    
    // Initialize pages (will be filled as we create pages in Step 3)
    void init() {
        renderer->clearScreen();
        
        // Register pages
        registerPage(PageType::ALGORITHM, new PageAlgorithm(config, synth, renderer));
        registerPage(PageType::LFO, new PageLFO(config, synth, renderer));
        registerPage(PageType::PITCH_ENV, new PagePitchEnv(config, synth, renderer));
        
        navigateTo(PageType::ALGORITHM);
        
        #ifdef DEBUG_TEENSY
        Serial.println(F("UIManager: Ready"));
        #endif
    }
    
    // Register a page in the manager
    void registerPage(PageType type, Page* page) {
        size_t index = static_cast<size_t>(type);
        if (index < pages.size()) {
            pages[index] = page;
            
            #ifdef DEBUG_TEENSY
            Serial.print(F("UIManager: Registered page "));
            Serial.println(index);
            #endif
        }
    }
    
    // Navigate to specific page
    void navigateTo(PageType newPageType) {
        size_t newIndex = static_cast<size_t>(newPageType);
        
        if (newIndex >= pages.size() || pages[newIndex] == nullptr) {
            #ifdef DEBUG_TEENSY
            Serial.print(F("UIManager: Page not found: "));
            Serial.println(newIndex);
            #endif
            return;
        }
        
        if (currentPage) {
            currentPage->exit();
        }
        
        currentPageType = newPageType;
        currentPage = pages[newIndex];
        
        if (currentPage) {
            renderer->clearScreen();
            currentPage->enter();
        }
        
        #ifdef DEBUG_TEENSY
        Serial.print(F("UIManager: Navigated to page "));
        Serial.println(newIndex);
        #endif
    }
    
    // Change sub-page (PREV/NEXT buttons)
    void changeSubPage(int8_t direction) {
        if (currentPage) {
            bool changed = currentPage->changeSubPage(direction);
            
            #ifdef DEBUG_TEENSY
            if (changed) {
                Serial.print(F("UIManager: SubPage -> "));
                Serial.println(currentPage->getCurrentSubPage());
            }
            #endif
        }
    }
    
    // Update interface (called in loop(), throttled to 20 FPS)
    void update() {
        unsigned long now = millis();
        
        if (now - lastUpdateTime < UPDATE_INTERVAL_MS) {
            return;
        }
        
        lastUpdateTime = now;
        
        if (currentPage) {
            currentPage->update();
        }
    }
    
    // Callback for encoder rotation (called from EncodersHandler)
    void onEncoderRotation(uint8_t encoderIndex, int8_t direction) {
        if (currentPage) {
            currentPage->handleEncoder(encoderIndex, direction);
        }
    }
    
    // Callback for button press (called from ButtonsHandler)
    // Page handles button first - if not handled, default navigation occurs
    void onButtonPress(uint8_t buttonIndex) {
        // Let page handle button first (important for Save page and special cases)
        if (currentPage && currentPage->handleButton(buttonIndex)) {
            return;  // Page handled it, don't do default navigation
        }
        
        // Default navigation behavior
        if (buttonIndex == 0) {
            changeSubPage(-1);  // PREV
            return;
        } else if (buttonIndex == 1) {
            changeSubPage(+1);  // NEXT
            return;
        }
        
        if (buttonIndex >= 2 && buttonIndex <= 14) {
            PageType targetPage = static_cast<PageType>(buttonIndex - 2);
            navigateTo(targetPage);
            return;
        }
    }
    
    // Callback for encoder button press (called from EncodersHandler)
    void onEncoderButtonPress(uint8_t encoderIndex) {
        if (currentPage) {
            currentPage->handleButton(ENCODER_BUTTON_OFFSET + encoderIndex);
        }
    }
    
    Renderer* getRenderer() {
        return renderer;
    }
    
    PageType getCurrentPageType() const {
        return currentPageType;
    }
    
    Page* getCurrentPage() {
        return currentPage;
    }
};

#endif // UI_MANAGER_H
