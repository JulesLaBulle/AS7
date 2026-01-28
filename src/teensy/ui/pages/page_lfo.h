#ifndef PAGE_LFO_H
#define PAGE_LFO_H

#include "../page.h"
#include "../widget_types.h"

// LFO Configuration Page
// Controls: Speed, Delay, PMD, AMD, Pitch Mod Sens, LFO Key Sync, OSC Key Sync, Waveform
// Layout:
//   Row 0: [Speed] [Delay] [PMD] [AMD]
//   Row 1: [PMS] [LFOKSync] [OSCKSync] [Wave]
class PageLFO : public Page {
public:
    PageLFO(SynthConfig* cfg, Synth* s, Renderer* r) 
        : Page(cfg, s, r) {}
    
    void enter() override {
        Page::enter();
        
        // Sync local copies with config
        if (config) {
            speed = config->lfoConfig.speed;
            delay = config->lfoConfig.delay;
            pmd = config->lfoConfig.pitchModDepth;
            amd = config->lfoConfig.ampModDepth;
            pms = config->lfoConfig.pitchModSens;
            lfoKeySync = config->lfoConfig.LFOKeySync;
            waveform = config->lfoConfig.waveform;
            oscKeySync = config->voiceConfig.operatorConfigs[0].OSCKeySync;
        }
    }
    
    void handleEncoder(uint8_t encoder, int8_t direction) override {
        if (!config || !synth) return;
        
        switch(encoder) {
            case 0:  // Speed
                speed = constrain(speed + direction, 0, 99);
                config->lfoConfig.speed = speed;
                dirtyWidget = 0;
                break;
                
            case 1:  // Delay
                delay = constrain(delay + direction, 0, 99);
                config->lfoConfig.delay = delay;
                dirtyWidget = 1;
                break;
                
            case 2:  // PMD (Pitch Mod Depth)
                pmd = constrain(pmd + direction, 0, 99);
                config->lfoConfig.pitchModDepth = pmd;
                dirtyWidget = 2;
                break;
                
            case 3:  // AMD (Amp Mod Depth)
                amd = constrain(amd + direction, 0, 99);
                config->lfoConfig.ampModDepth = amd;
                dirtyWidget = 3;
                break;
                
            case 4:  // Pitch Mod Sens
                pms = constrain(pms + direction, 0, 7);
                config->lfoConfig.pitchModSens = pms;
                dirtyWidget = 4;
                break;
                
            case 5:  // LFO Key Sync (toggle)
                lfoKeySync = !lfoKeySync;
                config->lfoConfig.LFOKeySync = lfoKeySync;
                dirtyWidget = 5;
                break;
                
            case 6:  // OSC Key Sync (toggle) - applies to all operators
                oscKeySync = !oscKeySync;
                synth->setOSCKeySync(oscKeySync);
                dirtyWidget = 6;
                break;
                
            case 7:  // Waveform
                waveform = (waveform + direction + 6) % 6;  // Wrap 0-5
                config->lfoConfig.waveform = waveform;
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
            renderer->drawHeader("LFO");
            renderer->drawWidgets(widgets, 8);
            fullRedraw = false;
        } else if (dirtyWidget >= 0 && dirtyWidget < 8) {
            renderer->updateWidgetValue(widgets[dirtyWidget]);
            dirtyWidget = -1;
        }
    }
    
private:
    // Local copies for display/editing
    uint8_t speed = 0;
    uint8_t delay = 0;
    uint8_t pmd = 0;
    uint8_t amd = 0;
    uint8_t pms = 0;
    bool lfoKeySync = false;
    bool oscKeySync = false;
    uint8_t waveform = 0;
    
    // Widget descriptors (declarative layout)
    // Row 0: Speed, Delay, PMD, AMD (positions 0-3)
    // Row 1: PMS, LFO Key Sync, OSC Key Sync, Waveform (positions 4-7)
    WidgetDescriptor widgets[8] = {
        WidgetDescriptor("Speed", WidgetType::KNOB, 0, &speed, 0, 99, 1, 1),
        WidgetDescriptor("Delay", WidgetType::KNOB, 1, &delay, 0, 99, 1, 1),
        WidgetDescriptor("Pitch Mod", "Depth", WidgetType::KNOB, 2, &pmd, 0, 99, 1, 1),
        WidgetDescriptor("Amp Mod", "Depth", WidgetType::KNOB, 3, &amd, 0, 99, 1, 1),
        WidgetDescriptor("Pitch Mod", "Sens", WidgetType::KNOB, 4, &pms, 0, 7, 1, 1),
        WidgetDescriptor("LFO", "Key Sync", WidgetType::TOGGLE, 5, &lfoKeySync, 0, 1, 1, 1),
        WidgetDescriptor("OSC", "Key Sync", WidgetType::TOGGLE, 6, &oscKeySync, 0, 1, 1, 1),
        WidgetDescriptor("Waveform", WidgetType::KNOB, 7, &waveform, 0, 5, 1, 1)  // For now: knob (later: waveform preview)
    };
};

#endif // PAGE_LFO_H
