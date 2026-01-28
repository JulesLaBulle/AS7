#ifndef PAGE_PITCH_ENV_H
#define PAGE_PITCH_ENV_H

#include "../page.h"
#include "../widget_types.h"

// Pitch Envelope Configuration Page
// Controls: 4 Rates (R1-R4) and 4 Levels (L1-L4)
// Layout:
//   Row 0: [R1] [R2] [R3] [R4]
//   Row 1: [L1] [L2] [L3] [L4]
class PagePitchEnv : public Page {
public:
    PagePitchEnv(SynthConfig* cfg, Synth* s, Renderer* r) 
        : Page(cfg, s, r) {}
    
    void enter() override {
        Page::enter();
        
        // Sync local copies with config
        if (config) {
            r1 = config->pitchEnvelopeConfig.r1;
            r2 = config->pitchEnvelopeConfig.r2;
            r3 = config->pitchEnvelopeConfig.r3;
            r4 = config->pitchEnvelopeConfig.r4;
            l1 = config->pitchEnvelopeConfig.l1;
            l2 = config->pitchEnvelopeConfig.l2;
            l3 = config->pitchEnvelopeConfig.l3;
            l4 = config->pitchEnvelopeConfig.l4;
        }
    }
    
    void handleEncoder(uint8_t encoder, int8_t direction) override {
        if (!config) return;
        
        switch(encoder) {
            case 0:  // R1
                r1 = constrain(r1 + direction, 0, 99);
                config->pitchEnvelopeConfig.r1 = r1;
                dirtyWidget = 0;
                break;
                
            case 1:  // R2
                r2 = constrain(r2 + direction, 0, 99);
                config->pitchEnvelopeConfig.r2 = r2;
                dirtyWidget = 1;
                break;
                
            case 2:  // R3
                r3 = constrain(r3 + direction, 0, 99);
                config->pitchEnvelopeConfig.r3 = r3;
                dirtyWidget = 2;
                break;
                
            case 3:  // R4
                r4 = constrain(r4 + direction, 0, 99);
                config->pitchEnvelopeConfig.r4 = r4;
                dirtyWidget = 3;
                break;
                
            case 4:  // L1
                l1 = constrain(l1 + direction, 0, 99);
                config->pitchEnvelopeConfig.l1 = l1;
                dirtyWidget = 4;
                break;
                
            case 5:  // L2
                l2 = constrain(l2 + direction, 0, 99);
                config->pitchEnvelopeConfig.l2 = l2;
                dirtyWidget = 5;
                break;
                
            case 6:  // L3
                l3 = constrain(l3 + direction, 0, 99);
                config->pitchEnvelopeConfig.l3 = l3;
                dirtyWidget = 6;
                break;
                
            case 7:  // L4
                l4 = constrain(l4 + direction, 0, 99);
                config->pitchEnvelopeConfig.l4 = l4;
                dirtyWidget = 7;
                break;
        }
    }
    
    bool handleButton(uint8_t button) override {
        (void)button;
        return false;
    }
    
    void update() override {
        if (!renderer || !config) return;
        
        if (fullRedraw) {
            renderer->clearScreen();
            renderer->drawHeader("PITCH ENVELOPE");
            renderer->drawWidgets(widgets, 8);
            fullRedraw = false;
        } else if (dirtyWidget >= 0 && dirtyWidget < 8) {
            renderer->updateWidgetValue(widgets[dirtyWidget]);
            dirtyWidget = -1;
        }
    }
    
private:
    // Local copies for display/editing
    uint8_t r1 = 0;
    uint8_t r2 = 0;
    uint8_t r3 = 0;
    uint8_t r4 = 0;
    uint8_t l1 = 50;
    uint8_t l2 = 50;
    uint8_t l3 = 50;
    uint8_t l4 = 50;
    
    // Widget descriptors (declarative layout)
    // Row 0: R1, R2, R3, R4 (positions 0-3)
    // Row 1: L1, L2, L3, L4 (positions 4-7)
    WidgetDescriptor widgets[8] = {
        WidgetDescriptor("Rate 1", WidgetType::KNOB, 0, &r1, 0, 99, 1, 1),
        WidgetDescriptor("Rate 2", WidgetType::KNOB, 1, &r2, 0, 99, 1, 1),
        WidgetDescriptor("Rate 3", WidgetType::KNOB, 2, &r3, 0, 99, 1, 1),
        WidgetDescriptor("Rate 4", WidgetType::KNOB, 3, &r4, 0, 99, 1, 1),
        WidgetDescriptor("Level 1", WidgetType::KNOB, 4, &l1, 0, 99, 1, 1),
        WidgetDescriptor("Level 2", WidgetType::KNOB, 5, &l2, 0, 99, 1, 1),
        WidgetDescriptor("Level 3", WidgetType::KNOB, 6, &l3, 0, 99, 1, 1),
        WidgetDescriptor("Level 4", WidgetType::KNOB, 7, &l4, 0, 99, 1, 1)
    };
};

#endif // PAGE_PITCH_ENV_H
