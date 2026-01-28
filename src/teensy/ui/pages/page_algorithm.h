#ifndef PAGE_ALGORITHM_H
#define PAGE_ALGORITHM_H

#include "../page.h"
#include "../renderer.h"
#include "../widget_types.h"
#include "../../../core/config.h"
#include "../../../core/synth.h"
#include "../../../core/connections.h"

// Algorithm page - displays and edits algorithm selection (1-32) and feedback (0-7)
// Layout: 2 knobs (positions 0 and 4) + algorithm diagram (positions 1-3, 5-7)
class PageAlgorithm : public Page {
private:
    uint8_t currentAlgorithm;  // 0-31 internal, displayed as 1-32
    uint8_t currentFeedback;   // 0-7
    
    // Widget descriptors (declarative layout)
    // Algorithm diagram spans 3 cols x 2 rows (positions 1,2,3 and 5,6,7)
    WidgetDescriptor widgets[3] = {
        WidgetDescriptor("Algorithm", WidgetType::LARGE_VALUE, 0, &currentAlgorithm, 1, 32, 1, 1),    // Position 0 (top-left)
        WidgetDescriptor("Feedback", WidgetType::KNOB, 4, &currentFeedback, 0, 7, 1, 1),      // Position 4 (bottom-left)
        WidgetDescriptor("", WidgetType::ALGORITHM_DIAGRAM, 1, nullptr, 0, 0, 3, 2)       // Spans 3x2 (rest of screen)
    };
    
public:
    PageAlgorithm(SynthConfig* cfg, Synth* syn, Renderer* rend)
        : Page(cfg, syn, rend), currentAlgorithm(0), currentFeedback(0) {}
    
    void enter() override {
        Page::enter();
        
        // Load current values from config
        if (config && config->voiceConfig.algorithm) {
            // Find which algorithm index matches current config
            for (size_t i = 0; i < Algorithms::NUM_ALGORITHMS; i++) {
                if (config->voiceConfig.algorithm == Algorithms::ALL_ALGORITHMS[i]) {
                    currentAlgorithm = i;
                    break;
                }
            }
        }
        currentFeedback = config->voiceConfig.feedback;
    }
    
    void update() override {
        if (!renderer || !config) return;
        
        if (fullRedraw) {
            renderer->clearScreen();
            renderer->drawHeader("ALGORITHM");
            
            // displayAlgo for showing 1-32 instead of 0-31
            uint8_t displayAlgo = currentAlgorithm + 1;
            widgets[0].valuePtr = &displayAlgo;
            renderer->drawWidgets(widgets, 3);
            widgets[0].valuePtr = &currentAlgorithm;
            
            fullRedraw = false;
        } else if (dirtyWidget >= 0 && dirtyWidget < 3) {
            // Update only changed widget
            if (dirtyWidget == 0) {
                // Algorithm widget needs display value (1-32)
                uint8_t displayAlgo = currentAlgorithm + 1;
                WidgetDescriptor temp = widgets[0];
                temp.valuePtr = &displayAlgo;
                renderer->updateWidgetValue(temp);
            } else {
                renderer->updateWidgetValue(widgets[dirtyWidget]);
            }
            dirtyWidget = -1;
        }
    }
    
    void handleEncoder(uint8_t encoderIndex, int8_t direction) override {
        if (encoderIndex == 0) {
            int16_t newAlgo = static_cast<int16_t>(currentAlgorithm) + direction;
            newAlgo = constrain(newAlgo, 0, 31);
            
            if (newAlgo != currentAlgorithm) {
                currentAlgorithm = static_cast<uint8_t>(newAlgo);
                synth->setAlgorithm(Algorithms::ALL_ALGORITHMS[currentAlgorithm]);
                dirtyWidget = 0;  // Also triggers diagram update (position 2)
            }
            
        } else if (encoderIndex == 4) {
            int16_t newFb = static_cast<int16_t>(currentFeedback) + direction;
            newFb = constrain(newFb, 0, 7);
            
            if (newFb != currentFeedback) {
                currentFeedback = static_cast<uint8_t>(newFb);
                synth->setFeedback(currentFeedback);
                dirtyWidget = 1;
            }
        }
    }
    
    bool handleButton(uint8_t buttonIndex) override {
        // No special button handling, let UIManager handle navigation
        (void)buttonIndex;
        return false;
    }
};

#endif // PAGE_ALGORITHM_H
