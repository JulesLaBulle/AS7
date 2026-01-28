#ifndef PAGE_OPERATOR_H
#define PAGE_OPERATOR_H

#include "../page.h"
#include "../renderer.h"
#include "../../../core/config.h"
#include "../../../core/synth.h"

// Page for editing a single operator (6 instances, one per operator)
// 3 sub-pages: Voice, Envelope, Level Scaling
// Sub-page is shared across all operator pages (static)
class PageOperator : public Page {
private:
    uint8_t operatorIndex;  // 0-5 for operators 1-6
    
    // Shared sub-page index across all operator instances
    static uint8_t sharedSubPage;  // 0=Voice, 1=Envelope, 2=Level Scaling
    
    // Local copies for display/editing (Voice page)
    uint8_t detune;
    uint8_t coarse;
    uint8_t fine;
    uint8_t level;
    bool fixedFreq;
    bool opOn;
    uint8_t waveform;
    
    // Envelope values
    uint8_t rate1, rate2, rate3, rate4;
    uint8_t level1, level2, level3, level4;
    
    // Level scaling values
    uint8_t ampModSens;
    uint8_t keyVelSens;
    uint8_t lDepth;
    uint8_t rDepth;
    uint8_t breakpoint;
    uint8_t lCurve;  // 0-3: -LIN, -EXP, +EXP, +LIN
    uint8_t rCurve;
    uint8_t rateScaling;
    
    // Widget descriptors for each sub-page
    // Voice sub-page (position 0)
    WidgetDescriptor voiceWidgets[8] = {
        {"Detune", WidgetType::KNOB, 0, &detune, 0, 14},        // Encoder 1
        {"Coarse", WidgetType::KNOB, 1, &coarse, 0, 31},        // Encoder 2
        {"Fine", WidgetType::KNOB, 2, &fine, 0, 99},            // Encoder 3
        {"Level", WidgetType::KNOB, 3, &level, 0, 99},          // Encoder 4
        {"Ratio", "Fixed", WidgetType::TOGGLE, 4, &fixedFreq, 0, 1}, // Encoder 5
        {"Wave", WidgetType::WAVEFORM_OSC, 5, &waveform, 0, 4}, // Encoder 6
        {"", WidgetType::KNOB, 6, nullptr, 0, 0},               // Unused
        {"OP On", WidgetType::TOGGLE, 7, &opOn, 0, 1}           // Encoder 8
    };
    
    // Envelope sub-page (position 1)
    WidgetDescriptor envelopeWidgets[8] = {
        {"Rate 1", WidgetType::KNOB, 0, &rate1, 0, 99},   // Encoder 1
        {"Rate 2", WidgetType::KNOB, 1, &rate2, 0, 99},   // Encoder 2
        {"Rate 3", WidgetType::KNOB, 2, &rate3, 0, 99},   // Encoder 3
        {"Rate 4", WidgetType::KNOB, 3, &rate4, 0, 99},   // Encoder 4
        {"Level 1", WidgetType::KNOB, 4, &level1, 0, 99}, // Encoder 5
        {"Level 2", WidgetType::KNOB, 5, &level2, 0, 99}, // Encoder 6
        {"Level 3", WidgetType::KNOB, 6, &level3, 0, 99}, // Encoder 7
        {"Level 4", WidgetType::KNOB, 7, &level4, 0, 99}  // Encoder 8
    };
    
    // Level Scaling sub-page (position 2)
    WidgetDescriptor levelScalingWidgets[8] = {
        {"Amp Mod", "Sens", WidgetType::KNOB, 0, &ampModSens, 0, 3},        // Encoder 1
        {"Key Vel", "Sens", WidgetType::KNOB, 1, &keyVelSens, 0, 7},        // Encoder 2
        {"Left", " Depth", WidgetType::KNOB, 2, &lDepth, 0, 99},           // Encoder 3
        {"Right", " Depth", WidgetType::KNOB, 3, &rDepth, 0, 99},           // Encoder 4
        {"Rate", "Scaling", WidgetType::KNOB, 4, &rateScaling, 0, 7},      // Encoder 5
        {"Left", "Curve", WidgetType::WAVEFORM_LEVEL_SCALING, 5, &lCurve, 0, 3}, // Encoder 6
        {"Right", "Curve", WidgetType::WAVEFORM_LEVEL_SCALING, 6, &rCurve, 0, 3}, // Encoder 7
        {"Break", "Point", WidgetType::KNOB, 7, &breakpoint, 0, 99}       // Encoder 8
    };
    
    // Grid layout constants (compressed like parameters page)
    static constexpr uint16_t GRID_Y_OFFSET = 80;
    static constexpr uint16_t GRID_WIDGET_HEIGHT = 120;
    
    void loadValuesFromConfig() {
        OperatorConfig& op = config->voiceConfig.operatorConfigs[operatorIndex];
        
        // Voice
        detune = op.frequency.detune;
        coarse = op.frequency.coarse;
        fine = op.frequency.fine;
        level = op.envelope.outputLevel;
        fixedFreq = op.frequency.fixedFrequency;
        opOn = op.on;
        waveform = op.waveform;
        
        // Envelope
        rate1 = op.envelope.r1;
        rate2 = op.envelope.r2;
        rate3 = op.envelope.r3;
        rate4 = op.envelope.r4;
        level1 = op.envelope.l1;
        level2 = op.envelope.l2;
        level3 = op.envelope.l3;
        level4 = op.envelope.l4;
        
        // Level Scaling
        ampModSens = op.ampModSens;
        keyVelSens = op.velocitySensitivity;
        lDepth = op.lvlSclLeftDepth;
        rDepth = op.lvlSclRightDepth;
        breakpoint = op.lvlSclBreakpoint;
        lCurve = op.lvlSclLeftCurve;
        rCurve = op.lvlSclRightCurve;
        rateScaling = op.envelope.rateScaling;
    }
    
    const char* getSubPageName() const {
        switch (subPage) {
            case 0: return "Voice";
            case 1: return "Envelope";
            case 2: return "Level Scaling";
            default: return "";
        }
    }
    
public:
    PageOperator(SynthConfig* cfg, Synth* syn, Renderer* rend, uint8_t opIndex)
        : Page(cfg, syn, rend), operatorIndex(opIndex),
          detune(7), coarse(1), fine(0), level(99), fixedFreq(false), opOn(true), waveform(0),
          rate1(99), rate2(99), rate3(99), rate4(99),
          level1(99), level2(99), level3(99), level4(0),
          ampModSens(0), keyVelSens(0), lDepth(0), rDepth(0), breakpoint(39),
          lCurve(0), rCurve(0), rateScaling(0) {
        
        loadValuesFromConfig();
    }
    
    virtual uint8_t getSubPageCount() const override {
        return 3;
    }
    
    // Override to make sub-page navigation circular
    virtual bool changeSubPage(int8_t direction) override {
        uint8_t maxSubPages = getSubPageCount();
        if (maxSubPages <= 1) return false;
        
        // Circular navigation
        int16_t newSubPage = static_cast<int16_t>(subPage) + direction;
        
        if (newSubPage < 0) {
            newSubPage = maxSubPages - 1;  // Wrap to last
        } else if (newSubPage >= maxSubPages) {
            newSubPage = 0;  // Wrap to first
        }
        
        subPage = static_cast<uint8_t>(newSubPage);
        sharedSubPage = subPage;  // Update shared state
        fullRedraw = true;
        dirtyWidget = -1;
        return true;
    }
    
    void enter() override {
        Page::enter();
        
        // Set current sub-page from shared state
        subPage = sharedSubPage;
        
        // Load values from config
        loadValuesFromConfig();
        fullRedraw = true;
        dirtyWidget = -1;
    }
    
    void exit() override {
        // Save current sub-page to shared state
        sharedSubPage = subPage;
        Page::exit();
    }
    
    void update() override {
        if (!renderer || !config) return;
        
        if (fullRedraw) {
            renderer->clearScreen();
            
            // Header with operator number
            char header[16];
            snprintf(header, sizeof(header), "OPERATOR %d", operatorIndex + 1);
            renderer->drawHeader(header);
            
            // Instruction text with sub-page name
            renderer->drawInstructionText(getSubPageName());
            
            // Draw widgets for current sub-page
            switch (subPage) {
                case 0: // Voice
                    renderer->drawWidgets(voiceWidgets, 8, GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
                case 1: // Envelope
                    renderer->drawWidgets(envelopeWidgets, 8, GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
                case 2: // Level Scaling
                    renderer->drawWidgets(levelScalingWidgets, 8, GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
            }
            
            fullRedraw = false;
        } else if (dirtyWidget >= 0 && dirtyWidget < 8) {
            // Lazy update: only redraw changed widget
            switch (subPage) {
                case 0:
                    renderer->updateWidgetValue(voiceWidgets[dirtyWidget], GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
                case 1:
                    renderer->updateWidgetValue(envelopeWidgets[dirtyWidget], GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
                case 2:
                    renderer->updateWidgetValue(levelScalingWidgets[dirtyWidget], GRID_Y_OFFSET, GRID_WIDGET_HEIGHT);
                    break;
            }
            dirtyWidget = -1;
        }
    }
    
    void handleEncoder(uint8_t encoderIndex, int8_t direction) override {
        OperatorConfig& op = config->voiceConfig.operatorConfigs[operatorIndex];
        
        switch (subPage) {
            case 0: // Voice
                switch (encoderIndex) {
                    case 0: // Detune
                        detune = constrain(detune + direction, 0, 14);
                        op.frequency.detune = detune;
                        dirtyWidget = 0;
                        break;
                    case 1: // Coarse
                        coarse = constrain(coarse + direction, 0, 31);
                        op.frequency.coarse = coarse;
                        dirtyWidget = 1;
                        break;
                    case 2: // Fine
                        fine = constrain(fine + direction, 0, 99);
                        op.frequency.fine = fine;
                        dirtyWidget = 2;
                        break;
                    case 3: // Level
                        level = constrain(level + direction, 0, 99);
                        op.envelope.outputLevel = level;
                        dirtyWidget = 3;
                        break;
                    case 4: // Ratio/Fixed toggle
                        fixedFreq = !fixedFreq;
                        op.frequency.fixedFrequency = fixedFreq;
                        dirtyWidget = 4;
                        break;
                    case 5: // OP On toggle
                        opOn = !opOn;
                        op.on = opOn;
                        dirtyWidget = 5;
                        break;
                    case 6: // Waveform
                        waveform = (waveform + direction + 5) % 5;
                        op.waveform = waveform;
                        dirtyWidget = 6;
                        break;
                }
                break;
                
            case 1: // Envelope
                switch (encoderIndex) {
                    case 0: rate1 = constrain(rate1 + direction, 0, 99); op.envelope.r1 = rate1; dirtyWidget = 0; break;
                    case 1: rate2 = constrain(rate2 + direction, 0, 99); op.envelope.r2 = rate2; dirtyWidget = 1; break;
                    case 2: rate3 = constrain(rate3 + direction, 0, 99); op.envelope.r3 = rate3; dirtyWidget = 2; break;
                    case 3: rate4 = constrain(rate4 + direction, 0, 99); op.envelope.r4 = rate4; dirtyWidget = 3; break;
                    case 4: level1 = constrain(level1 + direction, 0, 99); op.envelope.l1 = level1; dirtyWidget = 4; break;
                    case 5: level2 = constrain(level2 + direction, 0, 99); op.envelope.l2 = level2; dirtyWidget = 5; break;
                    case 6: level3 = constrain(level3 + direction, 0, 99); op.envelope.l3 = level3; dirtyWidget = 6; break;
                    case 7: level4 = constrain(level4 + direction, 0, 99); op.envelope.l4 = level4; dirtyWidget = 7; break;
                }
                break;
                
            case 2: // Level Scaling
                switch (encoderIndex) {
                    case 0: ampModSens = constrain(ampModSens + direction, 0, 3); op.ampModSens = ampModSens; dirtyWidget = 0; break;
                    case 1: keyVelSens = constrain(keyVelSens + direction, 0, 7); op.velocitySensitivity = keyVelSens; dirtyWidget = 1; break;
                    case 2: lDepth = constrain(lDepth + direction, 0, 99); op.lvlSclLeftDepth = lDepth; dirtyWidget = 2; break;
                    case 3: rDepth = constrain(rDepth + direction, 0, 99); op.lvlSclRightDepth = rDepth; dirtyWidget = 3; break;
                    case 4: breakpoint = constrain(breakpoint + direction, 0, 99); op.lvlSclBreakpoint = breakpoint; dirtyWidget = 4; break;
                    case 5: lCurve = (lCurve + direction + 4) % 4; op.lvlSclLeftCurve = lCurve; dirtyWidget = 5; break;
                    case 6: rCurve = (rCurve + direction + 4) % 4; op.lvlSclRightCurve = rCurve; dirtyWidget = 6; break;
                    case 7: rateScaling = constrain(rateScaling + direction, 0, 7); op.envelope.rateScaling = rateScaling; dirtyWidget = 7; break;
                }
                break;
        }
        
        // Apply changes to synth
        synth->configure(config);
    }
    
    bool handleButton(uint8_t buttonIndex) override {
        (void)buttonIndex;
        return false;
    }
};

// Initialize shared sub-page (starts at Voice)
uint8_t PageOperator::sharedSubPage = 0;

#endif // PAGE_OPERATOR_H
