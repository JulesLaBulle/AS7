#ifndef PAGE_PRESET_H
#define PAGE_PRESET_H

#include "../page.h"
#include "../renderer.h"
#include "../../../../src/core/config.h"
#include "../../../../src/core/synth.h"
#include "../../../../src/core/sysex.h"
#include "../../../../src/core/user_presets.h"
#include <vector>
#include <string>

// Preset selection page - lists presets from current bank (USER or ROM)
// Encoder 1: scroll/select, press to load preset
// Encoder 8 (USER only): delete selected preset
// On load: applies preset, navigates back to algorithm page
class PagePreset : public Page {
private:
    SysexHandler* sysex;
    UserPresetsHandler* userPresets;
    
    std::vector<std::string> presetNames;  // Current bank's presets
    bool isUserBank;                       // True if USER bank active
    uint8_t selectedIndex;                 // Currently highlighted preset
    uint8_t loadedIndex;                   // Currently loaded preset
    uint8_t scrollOffset;                  // First visible item index
    
    // For lazy update optimization
    uint8_t oldSelectedIndex;              // Previous selected index
    uint8_t oldScrollOffset;               // Previous scroll offset
    bool headerDrawn;                      // Track if header/instruction drawn
    
    static constexpr uint8_t VISIBLE_ITEMS = 8;
    
    void refreshPresetList() {
        presetNames.clear();
        
        if (isUserBank) {
            // USER bank: get from userPresets handler
            const auto& userNames = userPresets->getPresetNames();
            for (const auto& name : userNames) {
                presetNames.push_back(name);
            }
        } else {
            // ROM bank: get from sysex handler
            #ifdef PLATFORM_TEENSY
            const char* names[32];
            sysex->getAllPresetsNames(names);
            for (uint8_t i = 0; i < 32; i++) {
                presetNames.push_back(std::string(names[i], 10));  // DX7 names are 10 chars
            }
            #else
            auto names = sysex->getAllPresetsNames();
            for (const auto& name : names) {
                presetNames.push_back(name);
            }
            #endif
        }
    }
    
    void updateScrollOffset() {
        // Safety check
        if (presetNames.empty()) {
            scrollOffset = 0;
            return;
        }
        
        // If list is shorter than visible items, always start at 0
        if (presetNames.size() <= VISIBLE_ITEMS) {
            scrollOffset = 0;
            return;
        }
        
        // Block scrolling: show 8 items at a time
        // Calculate which block the selected item is in
        uint8_t blockIndex = selectedIndex / VISIBLE_ITEMS;
        uint8_t newScrollOffset = blockIndex * VISIBLE_ITEMS;
        
        // Clamp to valid range
        int16_t maxOffset = static_cast<int16_t>(presetNames.size()) - VISIBLE_ITEMS;
        if (newScrollOffset > maxOffset) {
            newScrollOffset = static_cast<uint8_t>(maxOffset);
        }
        
        scrollOffset = newScrollOffset;
    }
    
public:
    PagePreset(SynthConfig* cfg, Synth* syn, Renderer* rend, SysexHandler* sx, UserPresetsHandler* up)
        : Page(cfg, syn, rend), sysex(sx), userPresets(up),
          isUserBank(false), selectedIndex(0), loadedIndex(0), scrollOffset(0),
          oldSelectedIndex(0), oldScrollOffset(0), headerDrawn(false) {}
    
    void enter() override {
        Page::enter();
        
        headerDrawn = false;  // Reset for full redraw on enter
        
        // Determine which bank is active (check ROM first - ROM takes priority)
        if (sysex->isBankLoaded()) {
            // ROM bank is loaded
            isUserBank = false;
        } else {
            // No ROM bank, assume USER
            isUserBank = true;
        }
        
        // Refresh preset list
        refreshPresetList();
        
        // Start with first preset selected
        selectedIndex = 0;
        loadedIndex = 0;
        updateScrollOffset();
    }
    
    void update() override {
        if (!renderer) return;
        
        // Prepare C-string array (needed for both full and incremental draw)
        const char* items[128];  // Max 128 presets
        for (size_t i = 0; i < presetNames.size() && i < 128; i++) {
            items[i] = presetNames[i].c_str();
        }
        
        if (fullRedraw) {
            // Full redraw: clear everything and draw header/instruction
            renderer->clearScreen();
            renderer->drawHeader("PRESETS");
            
            // Instruction text varies based on bank type
            if (isUserBank) {
                renderer->drawInstructionText("Select with 1, Delete with 8");
            } else {
                renderer->drawInstructionText("Select with 1");
            }
            
            renderer->drawScrollableList(items, static_cast<uint8_t>(presetNames.size()),
                                        selectedIndex, loadedIndex, scrollOffset);
            
            headerDrawn = true;
            oldSelectedIndex = selectedIndex;
            oldScrollOffset = scrollOffset;
            fullRedraw = false;
            
        } else if (headerDrawn && (oldSelectedIndex != selectedIndex || oldScrollOffset != scrollOffset)) {
            // Incremental update: only redraw changed items
            renderer->updateScrollableListIncremental(items, static_cast<uint8_t>(presetNames.size()),
                                                     oldSelectedIndex, selectedIndex,
                                                     loadedIndex, oldScrollOffset, scrollOffset);
            
            oldSelectedIndex = selectedIndex;
            oldScrollOffset = scrollOffset;
        }
    }
    
    void handleEncoder(uint8_t encoderIndex, int8_t direction) override {
        if (encoderIndex == 0) {  // Encoder 1: scroll
            if (presetNames.empty()) return;
            
            int16_t newIndex = static_cast<int16_t>(selectedIndex) + direction;
            newIndex = constrain(newIndex, 0, static_cast<int16_t>(presetNames.size()) - 1);
            
            if (newIndex != selectedIndex) {
                selectedIndex = static_cast<uint8_t>(newIndex);
                updateScrollOffset();
                // No fullRedraw - lazy update will handle it in update()
            }
        }
    }
    
    bool handleButton(uint8_t buttonIndex) override {
        // Safety check: ensure presetNames is not empty and indices are valid
        if (presetNames.empty() || selectedIndex >= presetNames.size()) return false;
        
        if (buttonIndex == 100) {  // Encoder 1 button: load selected preset
            if (selectedIndex >= presetNames.size()) return false;
            
            bool success = false;
            
            if (isUserBank) {
                // Load USER preset
                if (userPresets->loadPreset(config, selectedIndex)) {
                    synth->configure(config);
                    loadedIndex = selectedIndex;
                    success = true;
                    
                    #ifdef DEBUG_TEENSY
                    Serial.print(F("User preset loaded: "));
                    Serial.println(presetNames[selectedIndex].c_str());
                    #endif
                    
                    fullRedraw = true;  // Force refresh to show loaded preset
                }
            } else {
                // Load ROM preset
                if (sysex->loadPreset(config, selectedIndex)) {
                    synth->configure(config);
                    loadedIndex = selectedIndex;
                    success = true;
                    
                    #ifdef DEBUG_TEENSY
                    Serial.print(F("ROM preset loaded: "));
                    Serial.println(presetNames[selectedIndex].c_str());
                    #endif
                    
                    fullRedraw = true;  // Force refresh to show loaded preset
                }
            }
            
            // Signal UIManager to navigate back to algorithm page
            return success;
            
        } else if (buttonIndex == 107 && isUserBank) {  // Encoder 8 button: delete (USER only)
            if (selectedIndex >= presetNames.size()) return false;
            
            // Delete selected user preset
            if (userPresets->deletePreset(selectedIndex)) {
                #ifdef DEBUG_TEENSY
                Serial.print(F("User preset deleted: "));
                Serial.println(presetNames[selectedIndex].c_str());
                #endif
                
                // Refresh list after deletion
                refreshPresetList();
                
                // Adjust selected index if needed
                if (selectedIndex >= presetNames.size() && selectedIndex > 0) {
                    selectedIndex--;
                }
                
                // Adjust loaded index if needed
                if (loadedIndex >= presetNames.size() && loadedIndex > 0) {
                    loadedIndex--;
                }
                
                updateScrollOffset();
                fullRedraw = true;
            }
        }
        
        return false;
    }
    
    // Set bank type (called by UIManager when entering from bank page)
    void setBankType(bool userBank) {
        isUserBank = userBank;
    }
};

#endif // PAGE_PRESET_H
