#ifndef PAGE_BANK_H
#define PAGE_BANK_H

#include "../page.h"
#include "../renderer.h"
#include "../../../../src/core/config.h"
#include "../../../../src/core/synth.h"
#include "../../../../src/core/sysex.h"
#include "../../../../src/core/user_presets.h"
#include <vector>
#include <string>

// Bank selection page - lists USER bank + all .syx banks from /presets
// Encoder 1: scroll/select, press to load bank
// On load: loads bank's first preset, navigates to preset page
class PageBank : public Page {
private:
    SysexHandler* sysex;
    UserPresetsHandler* userPresets;
    
    std::vector<std::string> bankNames;  // "USER" + all .syx banks
    uint8_t selectedIndex;               // Currently highlighted bank
    uint8_t loadedIndex;                 // Currently loaded bank (0 = USER, 1+ = ROM)
    uint8_t scrollOffset;                // First visible item index
    
    // For lazy update optimization
    uint8_t oldSelectedIndex;            // Previous selected index
    uint8_t oldScrollOffset;             // Previous scroll offset
    bool headerDrawn;                    // Track if header/instruction drawn
    
    static constexpr uint8_t VISIBLE_ITEMS = 7;
    
    void refreshBankList() {
        bankNames.clear();
        
        // First entry: USER
        bankNames.push_back("USER");
        
        // Then all .syx banks from sysex handler
        const auto& syxBanks = sysex->getBanksList();
        for (const auto& name : syxBanks) {
            bankNames.push_back(name);
        }
    }
    
    void updateScrollOffset() {
        // Safety check
        if (bankNames.empty()) {
            scrollOffset = 0;
            return;
        }
        
        // If list is shorter than visible items, always start at 0
        if (bankNames.size() <= VISIBLE_ITEMS) {
            scrollOffset = 0;
            return;
        }
        
        // Center selected item when possible (only for longer lists)
        int16_t centerOffset = static_cast<int16_t>(selectedIndex) - (VISIBLE_ITEMS / 2);
        
        // Clamp to valid range [0, maxOffset]
        if (centerOffset < 0) {
            centerOffset = 0;
        }
        
        int16_t maxOffset = static_cast<int16_t>(bankNames.size()) - VISIBLE_ITEMS;
        if (centerOffset > maxOffset) {
            centerOffset = maxOffset;
        }
        
        scrollOffset = static_cast<uint8_t>(centerOffset);
    }
    
public:
    PageBank(SynthConfig* cfg, Synth* syn, Renderer* rend, SysexHandler* sx, UserPresetsHandler* up)
        : Page(cfg, syn, rend), sysex(sx), userPresets(up), 
          selectedIndex(0), loadedIndex(1), scrollOffset(0),
          oldSelectedIndex(0), oldScrollOffset(0), headerDrawn(false) {}  // Default: ROM1A loaded (index 1)
    
    void enter() override {
        Page::enter();
        
        headerDrawn = false;  // Reset for full redraw on enter
        
        // Refresh bank list
        refreshBankList();
        
        // Try to find currently loaded bank (check ROM first - ROM takes priority)
        if (sysex->isBankLoaded()) {
            // ROM bank is loaded
            const char* currentBank = sysex->getBankName();
            
            for (size_t i = 1; i < bankNames.size(); i++) {  // Skip USER (index 0)
                if (bankNames[i] == currentBank) {
                    loadedIndex = static_cast<uint8_t>(i);
                    break;
                }
            }
        } else {
            // No ROM bank loaded, assume USER
            loadedIndex = 0;
        }
        
        selectedIndex = loadedIndex;
        updateScrollOffset();
    }
    
    void update() override {
        if (!renderer || bankNames.empty()) return;
        
        // Prepare C-string array with proper bounds
        const char* items[64];  // Max 64 banks (should be enough)
        uint8_t itemCount = static_cast<uint8_t>(bankNames.size());
        if (itemCount > 64) itemCount = 64;  // Safety clamp
        
        for (uint8_t i = 0; i < itemCount; i++) {
            items[i] = bankNames[i].c_str();
        }
        
        if (fullRedraw) {
            // Full redraw: clear everything and draw header/instruction
            renderer->clearScreen();
            renderer->drawHeader("BANKS");
            
            // Instruction text: add delete option for USER bank
            if (loadedIndex == 0) {
                renderer->drawInstructionText("Select with 1, Delete with 8");
            } else {
                renderer->drawInstructionText("Select with 1");
            }
            
            renderer->drawScrollableList(items, itemCount, 
                                        selectedIndex, loadedIndex, scrollOffset);
            
            headerDrawn = true;
            oldSelectedIndex = selectedIndex;
            oldScrollOffset = scrollOffset;
            fullRedraw = false;
            
        } else if (headerDrawn && (oldSelectedIndex != selectedIndex || oldScrollOffset != scrollOffset)) {
            // Incremental update: only redraw changed items
            renderer->updateScrollableListIncremental(items, itemCount,
                                                     oldSelectedIndex, selectedIndex,
                                                     loadedIndex, oldScrollOffset, scrollOffset);
            
            oldSelectedIndex = selectedIndex;
            oldScrollOffset = scrollOffset;
        }
    }
    
    void handleEncoder(uint8_t encoderIndex, int8_t direction) override {
        if (encoderIndex == 0) {  // Encoder 1: scroll
            if (bankNames.empty()) return;  // Safety check
            
            int16_t newIndex = static_cast<int16_t>(selectedIndex) + direction;
            newIndex = constrain(newIndex, 0, static_cast<int16_t>(bankNames.size()) - 1);
            
            if (newIndex != selectedIndex) {
                selectedIndex = static_cast<uint8_t>(newIndex);
                updateScrollOffset();
                // No fullRedraw - lazy update will handle it in update()
            }
        }
    }
    
    bool handleButton(uint8_t buttonIndex) override {
        if (buttonIndex == 100) {  // Encoder 1 button: load selected bank
            if (selectedIndex >= bankNames.size()) return false;
            
            if (selectedIndex == 0) {
                // Load USER bank
                if (userPresets->loadUserBank()) {
                    // Unload ROM bank (we're now in USER mode)
                    sysex->unloadBank();
                    
                    loadedIndex = 0;
                    
                    #ifdef DEBUG_TEENSY
                    Serial.println(F("USER bank loaded"));
                    #endif
                    
                    fullRedraw = true;  // Force refresh to show loaded bank
                    // Signal UIManager to navigate to preset page
                    return true;  // Let UIManager handle navigation
                }
            } else {
                // Load .syx bank
                std::string filename = "/presets/" + bankNames[selectedIndex] + ".syx";
                if (sysex->loadBank(filename.c_str())) {
                    // Unload user bank (we're now in ROM mode)
                    userPresets->unloadUserBank();
                    
                    // Load first preset
                    if (sysex->loadPreset(config, 0)) {
                        synth->configure(config);
                        loadedIndex = selectedIndex;
                        
                        #ifdef DEBUG_TEENSY
                        Serial.print(F("Bank loaded: "));
                        Serial.println(bankNames[selectedIndex].c_str());
                        #endif
                        
                        fullRedraw = true;  // Force refresh to show loaded bank
                        // Signal UIManager to navigate to preset page
                        return true;  // Let UIManager handle navigation
                    }
                }
            }
        }
        
        return false;
    }
    
    // Get currently loaded bank type (for UIManager logic)
    bool isUserBankLoaded() const {
        return loadedIndex == 0;
    }
};

#endif // PAGE_BANK_H
