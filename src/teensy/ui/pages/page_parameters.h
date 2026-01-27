#ifndef PAGE_PARAMETERS_H
#define PAGE_PARAMETERS_H

#include "../page.h"
#include "../renderer.h"
#include "../../../core/config.h"
#include "../../../core/synth.h"

// Page for editing global synthesizer parameters
// Grid layout (4x2):
//   [0:PitchMod] [1:AmpMod] [2:EGBias] [3:ModIntens]
//   [4:PitchBend] [5:unused] [6:unused] [7:MidiChan]
class PageParameters : public Page {
private:
    // Temporary values (not saved until user presses PARAMETERS button)
    bool pitchMod;
    bool ampMod;
    bool egBias;
    uint8_t modIntensity;
    uint8_t pitchBendRange;
    uint8_t midiChannel;
    
    // Widget descriptors (declarative layout)
    WidgetDescriptor widgets[8] = {
        // Row 0
        {"PitchMod", WidgetType::TOGGLE, 0, &pitchMod, 0, 1},
        {"AmpMod", WidgetType::TOGGLE, 1, &ampMod, 0, 1},
        {"EGBias", WidgetType::TOGGLE, 2, &egBias, 0, 1},
        {"ModIntens", WidgetType::KNOB, 3, &modIntensity, 0, 99},
        // Row 1
        {"PitchBend", WidgetType::KNOB, 4, &pitchBendRange, 0, 24},
        {"", WidgetType::KNOB, 5, nullptr, 0, 0},  // Unused
        {"", WidgetType::KNOB, 6, nullptr, 0, 0},  // Unused
        {"MidiChan", WidgetType::LARGE_VALUE, 7, &midiChannel, 0, 16}  // 0=OMNI
    };
    
public:
    PageParameters(SynthConfig* cfg, Synth* syn, Renderer* rend)
        : Page(cfg, syn, rend), 
          pitchMod(false), ampMod(false), egBias(false),
          modIntensity(0), pitchBendRange(12), midiChannel(1) {}
    
    void enter() override {
        Page::enter();
        
        // Load current values from synth
        pitchMod = synth->params.modWheelAssignment.pitchModDepth;
        ampMod = synth->params.modWheelAssignment.ampModDepth;
        egBias = synth->params.modWheelAssignment.egBias;
        modIntensity = synth->params.modWheelIntensity;
        pitchBendRange = synth->params.pitchBendRange;
        midiChannel = synth->params.midiChannel;
    }
    
    void update() override {
        if (!renderer || !config) return;
        
        // Custom layout: start grid below instruction text
        // Instruction text ends at approximately Y=80 (50 header + 5 margin + 16 text height + 9 spacing)
        // Available space: 320 - 80 = 240px for grid
        // Widget height: 240 / 2 rows = 120px per row
        static constexpr uint16_t GRID_Y_OFFSET = 80;
        static constexpr uint16_t GRID_WIDGET_HEIGHT = 120;
        
        if (fullRedraw) {
            renderer->clearScreen();
            renderer->drawHeader("PARAMETERS");
            renderer->drawInstructionText("Press PARAMETERS to save");
            renderer->drawWidgets(widgets, 8, GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
            fullRedraw = false;
        } else if (dirtyWidget >= 0 && dirtyWidget < 8) {
            renderer->updateWidgetValue(widgets[dirtyWidget], GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
            dirtyWidget = -1;
        }
    }
    
    void handleEncoder(uint8_t encoderIndex, int8_t direction) override {
        bool changed = false;
        
        switch (encoderIndex) {
            case 0:  // Pitch Mod toggle
                pitchMod = !pitchMod;
                synth->params.modWheelAssignment.pitchModDepth = pitchMod;
                dirtyWidget = 0;
                changed = true;
                break;
                
            case 1:  // Amp Mod toggle
                ampMod = !ampMod;
                synth->params.modWheelAssignment.ampModDepth = ampMod;
                dirtyWidget = 1;
                changed = true;
                break;
                
            case 2:  // EG Bias toggle
                egBias = !egBias;
                synth->params.modWheelAssignment.egBias = egBias;
                dirtyWidget = 2;
                changed = true;
                break;
                
            case 3:  // Mod Intensity
                {
                    int16_t newValue = static_cast<int16_t>(modIntensity) + direction;
                    newValue = constrain(newValue, 0, 99);
                    if (newValue != modIntensity) {
                        modIntensity = static_cast<uint8_t>(newValue);
                        synth->params.modWheelIntensity = modIntensity;
                        dirtyWidget = 3;
                        changed = true;
                    }
                }
                break;
                
            case 4:  // Pitch Bend Range
                {
                    int16_t newValue = static_cast<int16_t>(pitchBendRange) + direction;
                    newValue = constrain(newValue, 0, 24);
                    if (newValue != pitchBendRange) {
                        pitchBendRange = static_cast<uint8_t>(newValue);
                        synth->params.pitchBendRange = pitchBendRange;
                        dirtyWidget = 4;
                        changed = true;
                    }
                }
                break;
                
            case 7:  // MIDI Channel (encoder 8, position 7)
                {
                    int16_t newValue = static_cast<int16_t>(midiChannel) + direction;
                    newValue = constrain(newValue, 0, 16);
                    if (newValue != midiChannel) {
                        midiChannel = static_cast<uint8_t>(newValue);
                        synth->params.midiChannel = midiChannel;
                        dirtyWidget = 7;
                        changed = true;
                    }
                }
                break;
        }
        
        // Note: changes are applied immediately to synth->params
        // but not saved to file until user presses PARAMETERS button
        (void)changed;  // Suppress unused warning
    }
    
    bool handleButton(uint8_t buttonIndex) override {
        // Button 12 (PARAMETERS) saves to file
        if (buttonIndex == 12) {
            if (synth->saveParams()) {
                #ifdef DEBUG_TEENSY
                Serial.println(F("Parameters saved successfully"));
                #endif
                
                // Visual feedback: update instruction text only
                renderer->drawInstructionText("Saved!");
                delay(1000);
                renderer->drawInstructionText("Press PARAMETERS to save");
                return true;
            } else {
                #ifdef DEBUG_TEENSY
                Serial.println(F("Failed to save parameters"));
                #endif
                
                // Visual feedback: update instruction text only
                renderer->drawInstructionText("Save failed!");
                delay(1000);
                renderer->drawInstructionText("Press PARAMETERS to save");
                return true;
            }
        }
        
        return false;
    }
};

#endif // PAGE_PARAMETERS_H
