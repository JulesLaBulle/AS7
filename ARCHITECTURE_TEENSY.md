# 🏗️ ARCHITECTURE AS7 TEENSY - GUIDE COMPLET

**Date:** Janvier 2026  
**Version:** 1.0  
**Statut:** Planification

---

## 📋 TABLE DES MATIÈRES

1. [Vue d'ensemble](#vue-densemble)
2. [Structure de fichiers](#structure-de-fichiers)
3. [Flux de données](#flux-de-données)
4. [Architecture par couches](#architecture-par-couches)
5. [Main loop](#main-loop)
6. [Évolution de l'interface](#évolution-de-linterface)
7. [Mapping encoders/boutons](#mapping-encoders--boutons)
8. [Ordre d'implémentation](#ordre-dimplémentation)
9. [Points clés](#points-clés)

---

## 🎯 VUE D'ENSEMBLE

### Matériel prévu

- **Microcontrôleur:** Teensy 4.1 @ 600 MHz
- **Affichage:** Écran TFT SPI (ST7789/ILI9341)
- **Contrôles:**
  - 8 encodeurs rotatifs EC11 (via 3 MUX CD4051)
  - Boutons poussoirs (via 2 shift registers 74HC165)
- **Audio:** Sortie I2S/DAC
- **MIDI:** Entrée DIN via optocoupler 6N138

### Objectifs architecture

- ✅ **Modulaire** : Chaque fichier = 1 responsabilité
- ✅ **Évolutif** : Interface texte → graphique sans refonte
- ✅ **Testable** : Couches hardware mockables
- ✅ **Performant** : Event-driven, pas de polling inutile
- ✅ **Maintenable** : Séparation claire hardware/logique/UI

---

## 📁 STRUCTURE DE FICHIERS

```
src/
├── core/                      # ✅ Existant - Moteur FM (inchangé)
│   ├── synth.h
│   ├── operator.h
│   ├── envelope.h
│   ├── lfo.h
│   ├── sysex.h
│   ├── params.h
│   └── ...
│
├── teensy/
│   ├── main.cpp               # ✅ Existant - Point d'entrée
│   │
│   ├── hardware/              # 🆕 HAL - Abstraction matériel
│   │   ├── display.h          # TFT SPI (ST7789, ILI9341...)
│   │   ├── encoders.h         # 8 encoders via 3 MUX
│   │   ├── buttons.h          # Buttons via 2 shift registers
│   │   ├── midi_input.h       # MIDI DIN + optocoupler
│   │   └── audio_output.h     # DAC/I2S output
│   │
│   ├── ui/                    # 🆕 Interface utilisateur
│   │   ├── ui_manager.h       # Gère pages et navigation
│   │   ├── renderer.h         # Abstraction affichage (texte/graphique)
│   │   │
│   │   ├── pages/             # Pages individuelles
│   │   │   ├── page_base.h    # Interface commune
│   │   │   ├── page_home.h    # Page d'accueil
│   │   │   ├── page_preset.h  # Sélection preset
│   │   │   ├── page_operator.h # Édition opérateurs
│   │   │   ├── page_envelope.h # Édition enveloppes
│   │   │   ├── page_lfo.h     # Édition LFO
│   │   │   └── page_global.h  # Params globaux
│   │   │
│   │   └── widgets/           # Composants réutilisables
│   │       ├── menu.h         # Menu scrollable
│   │       ├── knob.h         # Knob virtuel (encoder)
│   │       ├── envelope_view.h # Visualisation envelope
│   │       └── vu_meter.h     # VU-meter
│   │
│   └── system/                # 🆕 Logique système
│       ├── event_manager.h    # Dispatch événements
│       ├── state_manager.h    # État global application
│       └── config.h           # Config Teensy (pins, etc.)
```

---

## 🔄 FLUX DE DONNÉES

```
┌─────────────────────────────────────────────────────────────┐
│                        MAIN LOOP                             │
│  (Polling hardware + Process audio + Update display)        │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
   ┌─────────┐        ┌──────────┐       ┌──────────┐
   │ HARDWARE│        │  SYSTEM  │       │    UI    │
   │   HAL   │───────▶│ MANAGERS │◀─────▶│ MANAGER  │
   └─────────┘        └──────────┘       └──────────┘
        │                   │                   │
        │                   │                   │
        ▼                   ▼                   ▼
   Encoders            StateManager         Current Page
   Buttons             EventManager         Renderer
   MIDI In             Synth Engine         Widgets
   Display             SysexHandler
   Audio Out           Params
```

---

## 🏛️ ARCHITECTURE PAR COUCHES

### 1️⃣ HARDWARE LAYER (HAL)

**Rôle:** Abstraire le matériel, fournir une API simple et indépendante.

#### **encoders.h**

Gère les 8 encodeurs via 3 multiplexeurs.

```cpp
// Gère les 8 encoders via 3 MUX (CD4051)
class EncoderManager {
private:
    // Pins MUX: S0, S1, S2 (sélection) + SIG (signal)
    uint8_t muxPins[3][4];  // 3 MUX × 4 pins
    int16_t positions[8];
    uint8_t lastStates[8];
    bool buttonStates[8];
    
public:
    void init(uint8_t mux1Pins[4], uint8_t mux2Pins[4], uint8_t mux3Pins[4]);
    void scan();  // Appelé chaque ~1ms dans main loop
    
    int8_t getDelta(uint8_t encoderIndex);  // Retourne -1, 0, +1
    bool wasPressed(uint8_t encoderIndex);  // Click bouton encodeur
    void resetDelta(uint8_t encoderIndex);  // Clear après lecture
};
```

**Fonctionnement:**
- Scan séquentiel des 8 encodeurs via MUX
- Détection edges (phase A/B) pour direction
- Debouncing des boutons intégrés

---

#### **buttons.h**

Gère les boutons via shift registers en cascade.

```cpp
// Gère les boutons via shift registers (74HC165)
class ButtonManager {
private:
    uint8_t pinLoad;    // Parallel load (PL)
    uint8_t pinClock;   // Clock (CLK)
    uint8_t pinData;    // Serial data (Q7)
    
    uint16_t currentState = 0;   // État actuel (16 bits = 2 registres)
    uint16_t previousState = 0;
    uint32_t pressTime[16];      // Pour long press detection
    
public:
    void init(uint8_t load, uint8_t clock, uint8_t data);
    void scan();  // Lit les 2 shift registers
    
    bool isPressed(uint8_t buttonIndex);
    bool wasJustPressed(uint8_t buttonIndex);   // Edge rising
    bool wasJustReleased(uint8_t buttonIndex);  // Edge falling
    bool wasLongPressed(uint8_t buttonIndex);   // > 500ms
};
```

**Fonctionnement:**
- Lecture série des 16 boutons (2×8 bits)
- Edge detection pour événements précis
- Long press pour actions alternatives

---

#### **midi_input.h**

Gère l'entrée MIDI via DIN + optocoupler.

```cpp
// Gère MIDI via DIN + optocoupler (6N138)
class MIDIInput {
private:
    HardwareSerial* serial;  // Serial1 ou Serial2
    uint8_t buffer[3];
    uint8_t bufferIndex = 0;
    
public:
    struct MIDIMessage {
        enum Type { NOTE_ON, NOTE_OFF, CC, PITCH_BEND, NONE };
        Type type;
        uint8_t channel;
        uint8_t data1;  // Note/CC number
        uint8_t data2;  // Velocity/CC value
    };
    
    void init(HardwareSerial* ser = &Serial1);
    void poll();  // Lit messages MIDI disponibles
    
    bool available();
    MIDIMessage read();
};
```

**Fonctionnement:**
- UART à 31250 baud (standard MIDI)
- Parsing messages 3 bytes (status + 2 data)
- Queue interne pour buffering

---

#### **display.h**

Abstraction écran TFT SPI.

```cpp
// Abstraction écran TFT SPI (ST7789 / ILI9341)
class Display {
private:
    // Bibliothèque sous-jacente (Adafruit_ST7789, ILI9341_t3n, etc.)
    TFT_Driver* tft;
    
    uint16_t width;
    uint16_t height;
    bool doubleBuffer = false;
    
public:
    void init(uint8_t cs, uint8_t dc, uint8_t rst);
    void clear(uint16_t color = BLACK);
    void setRotation(uint8_t rotation);
    
    // Primitives dessin
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    
    // Buffering pour éviter flicker
    void swapBuffers();
    
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
};
```

**Fonctionnement:**
- Wrapping autour d'une lib TFT (Adafruit, ILI9341_t3n...)
- API unifiée indépendante du driver sous-jacent
- Double buffering optionnel pour animations fluides

---

#### **audio_output.h**

Sortie audio via DAC ou I2S.

```cpp
// Sortie audio DAC/I2S
class AudioOutput {
private:
    Synth* synth;
    
    // Audio callback (appelé par interrupt)
    static void fillBuffer(int16_t* buffer, size_t samples);
    
public:
    void init(Synth* synthPtr);
    void start();
    void stop();
    
    void setVolume(float volume);  // 0.0 - 1.0
    float getCPUUsage();           // % CPU audio
};
```

**Fonctionnement:**
- Utilise Teensy Audio Library ou I2S direct
- Callback interrupt pour remplir buffer
- Conversion float → int16 avec clipping

---

### 2️⃣ SYSTEM LAYER

**Rôle:** Logique métier, gestion d'état global.

#### **event_manager.h**

Hub central de distribution d'événements.

```cpp
// Hub central d'événements
class EventManager {
public:
    struct Event {
        enum Type {
            NONE,
            ENCODER_TURN,      // Un encodeur a tourné
            ENCODER_PRESS,     // Bouton encodeur cliqué
            BUTTON_PRESS,      // Bouton poussoir pressé
            BUTTON_LONG_PRESS, // Bouton maintenu
            MIDI_NOTE_ON,      // Note MIDI reçue
            MIDI_NOTE_OFF,
            MIDI_CC,           // Control Change
            MIDI_PITCH_BEND
        };
        
        Type type = NONE;
        uint8_t index;   // Quel encodeur/bouton
        int16_t value;   // Delta encodeur ou valeur MIDI
        uint8_t channel; // Canal MIDI
    };
    
private:
    EncoderManager* encoders;
    ButtonManager* buttons;
    MIDIInput* midi;
    
    static const size_t QUEUE_SIZE = 32;
    Event eventQueue[QUEUE_SIZE];
    size_t queueHead = 0;
    size_t queueTail = 0;
    
public:
    void init(EncoderManager* enc, ButtonManager* btn, MIDIInput* mid);
    
    // Collecte tous les événements hardware
    void update();
    
    // Lecture événements (FIFO)
    bool pollEvent(Event& e);
    bool hasEvents() const;
    void clearEvents();
};
```

**Fonctionnement:**
1. `update()` appelé chaque frame
2. Scan tous les périphériques
3. Génère événements et les met en queue
4. UI/System consomment la queue

---

#### **state_manager.h**

État global de l'application.

```cpp
// État global application
class StateManager {
public:
    // Moteur audio
    Synth synth;
    SysexHandler sysex;
    
    // Configuration courante
    SynthConfig currentConfig;
    Params globalParams;
    
    // État UI
    uint8_t currentPreset = 0;
    uint8_t currentBank = 0;
    uint8_t editingOperator = 0;  // 0-5
    bool isDirty = false;         // Config modifiée non sauvée
    bool isPlaying = false;       // Notes actives
    
    // Stats
    float cpuUsage = 0.0f;
    uint8_t activeVoices = 0;
    
public:
    void init();
    
    // Gestion presets
    bool loadPreset(uint8_t index);
    bool savePreset(uint8_t index);
    bool loadBank(const char* filename);
    
    // Application changements
    void applyConfig();
    void revertChanges();
    
    // MIDI
    void handleNoteOn(uint8_t note, uint8_t velocity);
    void handleNoteOff(uint8_t note);
    void handleCC(uint8_t cc, uint8_t value);
    void handlePitchBend(int16_t value);
    
    // Helpers
    bool hasUnsavedChanges() const { return isDirty; }
    const char* getPresetName() const;
};
```

**Fonctionnement:**
- Source unique de vérité (Single Source of Truth)
- Toutes les pages accèdent au même état
- Pas de duplication de données

---

### 3️⃣ UI LAYER

**Rôle:** Navigation, affichage, interaction utilisateur.

#### **ui_manager.h**

Gestionnaire de pages et navigation.

```cpp
// Gère navigation entre pages
class UIManager {
public:
    enum PageID {
        PAGE_HOME,
        PAGE_PRESET,
        PAGE_OPERATOR,
        PAGE_ENVELOPE,
        PAGE_LFO,
        PAGE_GLOBAL,
        PAGE_SAVE,
        NUM_PAGES
    };
    
private:
    Display* display;
    Renderer* renderer;
    StateManager* state;
    
    PageBase* pages[NUM_PAGES];
    PageBase* currentPage = nullptr;
    PageID currentPageID = PAGE_HOME;
    
    bool needsRedraw = true;
    uint32_t lastUpdateTime = 0;
    
public:
    void init(Display* disp, StateManager* st);
    
    // Navigation
    void switchPage(PageID id);
    PageID getCurrentPage() const { return currentPageID; }
    
    // Event handling
    void handleEvent(const EventManager::Event& e);
    
    // Rendering
    void update();  // Appelé à 60 FPS
    void forceRedraw() { needsRedraw = true; }
};
```

**Fonctionnement:**
1. Maintient tableau de toutes les pages
2. Route événements vers page active
3. Gère transitions (onExit/onEnter)
4. Contrôle fréquence de redraw (60 FPS)

---

#### **renderer.h**

Abstraction affichage (texte simple → graphique complexe).

```cpp
// Abstraction affichage
class Renderer {
private:
    Display* display;
    
    // Style config
    uint16_t colorFg = WHITE;
    uint16_t colorBg = BLACK;
    uint16_t colorAccent = CYAN;
    uint8_t fontSize = 1;
    
public:
    void init(Display* disp);
    
    // ===== VERSION 1: TEXTE SIMPLE =====
    void drawParameter(int16_t x, int16_t y, const char* name, int16_t value);
    void drawMenu(int16_t x, int16_t y, const char* items[], uint8_t count, uint8_t selected);
    void drawHeader(const char* title);
    void drawFooter(const char* text);
    void drawValue(int16_t x, int16_t y, const char* label, int16_t value, int16_t min, int16_t max);
    
    // ===== VERSION 2: GRAPHIQUE (à ajouter plus tard) =====
    void drawKnob(int16_t x, int16_t y, int16_t value, int16_t min, int16_t max, const char* label);
    void drawEnvelope(int16_t x, int16_t y, const EnvelopeConfig& env);
    void drawWaveform(int16_t x, int16_t y, const float* samples, size_t count);
    void drawVUMeter(int16_t x, int16_t y, float level);
    void drawAlgorithm(int16_t x, int16_t y, const AlgorithmConfig* algo);
    
    // Style
    void setColors(uint16_t fg, uint16_t bg, uint16_t accent);
};
```

**Avantage clé:**
- Pages utilisent `Renderer` au lieu de `Display` directement
- Changer de texte à graphique = modifier `Renderer` uniquement
- Pages restent **identiques** !

---

#### **page_base.h**

Interface commune à toutes les pages.

```cpp
// Interface commune à toutes les pages
class PageBase {
protected:
    StateManager* state = nullptr;
    Renderer* renderer = nullptr;
    
public:
    virtual ~PageBase() = default;
    
    void setContext(StateManager* st, Renderer* rend) {
        state = st;
        renderer = rend;
    }
    
    // Lifecycle
    virtual void onEnter() = 0;    // Appelé à l'entrée sur page
    virtual void onExit() = 0;     // Appelé à la sortie
    
    // Event handling
    virtual void onEncoderTurn(uint8_t index, int8_t delta) = 0;
    virtual void onEncoderPress(uint8_t index) = 0;
    virtual void onButtonPress(uint8_t index) = 0;
    virtual void onButtonLongPress(uint8_t index) = 0;
    
    // Rendering
    virtual void draw() = 0;
    virtual void update(uint32_t deltaTime) = 0;  // Pour animations
};
```

---

#### **page_preset.h** (exemple concret)

Page de sélection de presets.

```cpp
class PagePreset : public PageBase {
private:
    uint8_t selectedIndex = 0;
    uint8_t scrollOffset = 0;
    const uint8_t VISIBLE_ITEMS = 8;
    
public:
    void onEnter() override {
        selectedIndex = state->currentPreset;
        scrollOffset = selectedIndex > VISIBLE_ITEMS/2 ? selectedIndex - VISIBLE_ITEMS/2 : 0;
    }
    
    void onExit() override {
        // Rien à faire
    }
    
    void onEncoderTurn(uint8_t enc, int8_t delta) override {
        if (enc == 0) {  // Encoder principal = navigation
            int16_t newIndex = selectedIndex + delta;
            if (newIndex < 0) newIndex = 0;
            if (newIndex >= 32) newIndex = 31;
            selectedIndex = newIndex;
            
            // Auto-scroll
            if (selectedIndex < scrollOffset) {
                scrollOffset = selectedIndex;
            } else if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
                scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
            }
        }
    }
    
    void onEncoderPress(uint8_t enc) override {
        if (enc == 0) {
            // Charger preset sélectionné
            state->loadPreset(selectedIndex);
        }
    }
    
    void onButtonPress(uint8_t btn) override {
        if (btn == BTN_HOME) {
            // Retour page home (géré par UIManager)
        }
    }
    
    void onButtonLongPress(uint8_t btn) override {
        // Non utilisé sur cette page
    }
    
    void draw() override {
        renderer->drawHeader("SELECT PRESET");
        
        // Liste presets (zéro-copy sur Teensy!)
        const char* names[32];
        state->sysex.getAllPresetsNames(names);
        
        // Affiche presets visibles
        for (uint8_t i = 0; i < VISIBLE_ITEMS; ++i) {
            uint8_t presetIndex = scrollOffset + i;
            if (presetIndex >= 32) break;
            
            int16_t y = 20 + i * 12;
            bool isSelected = (presetIndex == selectedIndex);
            
            if (isSelected) {
                renderer->fillRect(0, y, 240, 12, BLUE);
            }
            
            renderer->drawText(5, y + 2, names[presetIndex], 
                             isSelected ? WHITE : GRAY);
        }
        
        // Scrollbar
        if (scrollOffset > 0 || scrollOffset + VISIBLE_ITEMS < 32) {
            float scrollPercent = (float)scrollOffset / (32 - VISIBLE_ITEMS);
            int16_t scrollY = 20 + scrollPercent * (VISIBLE_ITEMS * 12);
            renderer->fillRect(235, scrollY, 5, 20, WHITE);
        }
        
        renderer->drawFooter("Turn:Select | Press:Load");
    }
    
    void update(uint32_t deltaTime) override {
        // Pas d'animation sur cette page
    }
};
```

---

## 🔁 MAIN LOOP

Structure de la boucle principale.

```cpp
// main.cpp - Structure complète
#define DEBUG_TEENSY

#include <Arduino.h>
#include <SD.h>

// Hardware
#include "hardware/display.h"
#include "hardware/encoders.h"
#include "hardware/buttons.h"
#include "hardware/midi_input.h"
#include "hardware/audio_output.h"

// System
#include "system/event_manager.h"
#include "system/state_manager.h"
#include "system/config.h"

// UI
#include "ui/ui_manager.h"
#include "ui/renderer.h"

// Core
#include "../core/lut.h"

// ===== OBJETS GLOBAUX =====
Display display;
EncoderManager encoders;
ButtonManager buttons;
MIDIInput midi;
AudioOutput audio;

EventManager events;
StateManager state;

Renderer renderer;
UIManager ui;

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
    
    Serial.println(F("AS7 Initializing..."));
    
    // 1. Init core
    LUT::init();
    
    // 2. Init hardware
    display.init(TFT_CS, TFT_DC, TFT_RST);
    encoders.init(/* pins MUX */);
    buttons.init(BTN_LOAD, BTN_CLOCK, BTN_DATA);
    midi.init(&Serial1);
    
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println(F("ERROR: SD card failed!"));
        display.drawText(10, 10, "SD CARD ERROR", RED);
        while (1);
    }
    
    // 3. Init système
    state.init();
    events.init(&encoders, &buttons, &midi);
    
    // 4. Init UI
    renderer.init(&display);
    ui.init(&display, &state);
    ui.switchPage(UIManager::PAGE_HOME);
    
    // 5. Init audio (dernier car lance interrupts)
    audio.init(&state.synth);
    audio.start();
    
    Serial.println(F("AS7 Ready!"));
}

// ===== LOOP =====
void loop() {
    static uint32_t lastUpdate = 0;
    static uint32_t lastUIUpdate = 0;
    uint32_t now = micros();
    
    // 1. COLLECTER ÉVÉNEMENTS HARDWARE (chaque frame)
    events.update();
    
    // 2. TRAITER ÉVÉNEMENTS
    EventManager::Event e;
    while (events.pollEvent(e)) {
        
        // MIDI events → direct au synth (low latency)
        if (e.type == EventManager::Event::MIDI_NOTE_ON) {
            state.handleNoteOn(e.index, e.value);
        }
        else if (e.type == EventManager::Event::MIDI_NOTE_OFF) {
            state.handleNoteOff(e.index);
        }
        else if (e.type == EventManager::Event::MIDI_CC) {
            state.handleCC(e.index, e.value);
        }
        else if (e.type == EventManager::Event::MIDI_PITCH_BEND) {
            state.handlePitchBend(e.value);
        }
        
        // UI events → UIManager
        else {
            ui.handleEvent(e);
        }
    }
    
    // 3. GÉNÉRER AUDIO
    // → Géré automatiquement par interrupt callback dans audio_output.h
    
    // 4. METTRE À JOUR UI (60 FPS = ~16.6ms)
    if (now - lastUIUpdate > 16666) {
        ui.update();
        lastUIUpdate = now;
    }
    
    // 5. STATS (1 Hz)
    if (now - lastUpdate > 1000000) {
        state.cpuUsage = audio.getCPUUsage();
        lastUpdate = now;
    }
}
```

---

## 🎨 ÉVOLUTION DE L'INTERFACE

### Phase 1 : Texte simple

**Avantages:**
- Implémentation rapide (~1 semaine)
- Debugging facile
- Fonctionnel immédiatement

**Exemple:**
```
┌─────────────────────┐
│  SELECT PRESET      │
├─────────────────────┤
│  > BRASS 1          │
│    STRINGS          │
│    E.PIANO 1        │
│    FLUTE            │
│    BASS 1           │
└─────────────────────┘
```

```cpp
// Renderer version texte (simple)
void Renderer::drawParameter(x, y, name, value) {
    display->drawText(x, y, name, WHITE);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    display->drawText(x + 100, y, buf, CYAN);
}
```

---

### Phase 2 : Interface graphique custom

**Ajouts:**
- Knobs virtuels animés
- Visualisation enveloppes temps réel
- Diagramme algorithme
- VU-meters

**Exemple:**
```
┌─────────────────────────────────────┐
│  OPERATOR 1                   □ ON  │
├─────────────────────────────────────┤
│                                     │
│   ┌───┐  ┌───┐  ┌───┐  ┌───┐      │
│   │ ◷ │  │ ◶ │  │ ◴ │  │ ◵ │      │
│   └───┘  └───┘  └───┘  └───┘      │
│   RATE   LEVEL  DETUNE COARSE     │
│                                     │
│   ENV: ┌─┐                         │
│        │ └─────┐                   │
│        │       └───────            │
│        ▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤             │
└─────────────────────────────────────┘
```

```cpp
// Renderer version graphique (avancée)
void Renderer::drawKnob(x, y, value, min, max, label) {
    // Dessine cercle
    display->drawCircle(x, y, 20, WHITE);
    
    // Calcul angle (-135° à +135°)
    float normalized = (float)(value - min) / (max - min);
    float angle = -135 + (normalized * 270);
    float rad = angle * DEG_TO_RAD;
    
    // Dessine aiguille
    int16_t x1 = x + cos(rad) * 15;
    int16_t y1 = y + sin(rad) * 15;
    display->drawLine(x, y, x1, y1, CYAN);
    
    // Dessine point central
    display->fillCircle(x, y, 3, CYAN);
    
    // Label
    int16_t labelX = x - (strlen(label) * 3);
    display->drawText(labelX, y + 25, label, WHITE);
    
    // Valeur
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    int16_t valueX = x - (strlen(buf) * 3);
    display->drawText(valueX, y + 35, buf, GRAY);
}
```

**Avantage clé:** Les pages n'ont **pas besoin de changer** ! Elles appellent juste `renderer->drawKnob(...)` et ça marche dans les deux versions.

---

## 🎛️ MAPPING ENCODERS / BOUTONS

### Encoders (8)

| Encoder | Fonction principale | Fonction shift |
|---------|---------------------|----------------|
| **ENC0** | Navigation menu (scroll) | Bank select |
| **ENC1** | Operator 1 / Param 1 | Fine adjust |
| **ENC2** | Operator 2 / Param 2 | Fine adjust |
| **ENC3** | Operator 3 / Param 3 | Fine adjust |
| **ENC4** | Operator 4 / Param 4 | Fine adjust |
| **ENC5** | Operator 5 / Param 5 | Fine adjust |
| **ENC6** | Operator 6 / Param 6 | Fine adjust |
| **ENC7** | Master volume / Param 7 | - |

### Boutons (exemple 16 boutons)

| Bouton | Fonction | Long press |
|--------|----------|------------|
| **BTN_HOME** | Retour page home | - |
| **BTN_PRESET** | Page sélection preset | Bank manager |
| **BTN_OPERATOR** | Page édition operator | - |
| **BTN_ENVELOPE** | Page édition envelope | - |
| **BTN_LFO** | Page LFO | - |
| **BTN_GLOBAL** | Paramètres globaux | Settings |
| **BTN_SAVE** | Sauvegarder changements | Save as... |
| **BTN_LOAD** | Charger preset | Reload |
| **BTN_SHIFT** | Modificateur (+ encoder) | - |
| **BTN_PANIC** | All notes off | Reset synth |
| **BTN_1-6** | Sélection directe OP 1-6 | Toggle on/off |

---

## 🚀 ORDRE D'IMPLÉMENTATION RECOMMANDÉ

### Sprint 1 : Foundation (1-2 semaines)
- [x] ✅ Core audio engine (déjà fait!)
- [x] ✅ SysexHandler avec zero-copy (fait!)
- [ ] 🔲 `system/config.h` - Pins hardware
- [ ] 🔲 `hardware/display.h` - TFT basic
- [ ] 🔲 `hardware/buttons.h` - Shift registers
- [ ] 🔲 Test: Afficher texte + lire boutons

### Sprint 2 : Input & Events (1 semaine)
- [ ] 🔲 `hardware/encoders.h` - MUX + EC11
- [ ] 🔲 `system/event_manager.h` - Queue événements
- [ ] 🔲 `system/state_manager.h` - État global
- [ ] 🔲 Test: Encodeurs → Serial debug

### Sprint 3 : UI Base (1-2 semaines)
- [ ] 🔲 `ui/renderer.h` - Texte simple
- [ ] 🔲 `ui/page_base.h` - Interface
- [ ] 🔲 `ui/ui_manager.h` - Navigation
- [ ] 🔲 `ui/pages/page_home.h` - Page accueil
- [ ] 🔲 Test: Navigation fonctionnelle

### Sprint 4 : Preset Management (1 semaine)
- [ ] 🔲 `ui/pages/page_preset.h` - Liste presets
- [ ] 🔲 Intégration `getAllPresetsNames()`
- [ ] 🔲 Chargement preset → synth
- [ ] 🔲 Test: Sélection + son

### Sprint 5 : MIDI Input (1 semaine)
- [ ] 🔲 `hardware/midi_input.h` - Parser MIDI
- [ ] 🔲 Intégration events MIDI
- [ ] 🔲 Note on/off → synth
- [ ] 🔲 Test: Jouer notes MIDI

### Sprint 6 : Audio Output (1-2 semaines)
- [ ] 🔲 `hardware/audio_output.h` - I2S/DAC
- [ ] 🔲 Callback interrupt → synth.process()
- [ ] 🔲 Buffer management
- [ ] 🔲 Test: **PREMIER SON !** 🎉

### Sprint 7 : Editing Pages (2-3 semaines)
- [ ] 🔲 `ui/pages/page_operator.h` - Édition 6 ops
- [ ] 🔲 `ui/pages/page_envelope.h` - ADSR
- [ ] 🔲 `ui/pages/page_lfo.h` - LFO params
- [ ] 🔲 `ui/pages/page_global.h` - Params globaux
- [ ] 🔲 Test: Édition complète preset

### Sprint 8 : Save/Load (1 semaine)
- [ ] 🔲 Sauvegarde presets SD
- [ ] 🔲 Page "Save As"
- [ ] 🔲 Gestion isDirty
- [ ] 🔲 Test: Persistence

### Sprint 9 : Graphics V2 (optionnel, 2-3 semaines)
- [ ] 🔲 `renderer.h` - Knobs graphiques
- [ ] 🔲 Visualisation enveloppes
- [ ] 🔲 Diagramme algorithme
- [ ] 🔲 VU-meters
- [ ] 🔲 Animations

---

## 💡 POINTS CLÉS

### 1. Séparation Hardware / Logic / UI
```
Hardware change (autre écran) → modifier display.h UNIQUEMENT
Logic change (algo synth)      → modifier core/* UNIQUEMENT
UI change (nouveau layout)     → modifier renderer.h UNIQUEMENT
```

### 2. Event-Driven Architecture
- ❌ **Mauvais:** Pages qui `poll()` les encodeurs directement
- ✅ **Bon:** EventManager → queue → UIManager → Page

Pourquoi ? Découplage, testabilité, réactivité.

### 3. Single Responsibility
- 1 fichier = 1 responsabilité
- Facile à débugger (bug display ? → `display.h`)
- Facile à tester (mock `Display` pour tester `Renderer`)

### 4. Abstraction Display via Renderer
```cpp
// Page ne sait PAS si c'est texte ou graphique
void PageOperator::draw() {
    renderer->drawKnob(10, 20, value, 0, 99, "Level");
}

// Renderer décide comment dessiner
void Renderer::drawKnob(...) {
    #ifdef SIMPLE_UI
        drawParameter(x, y, label, value);  // Texte
    #else
        drawGraphicalKnob(x, y, value);     // Graphique
    #endif
}
```

### 5. StateManager = Single Source of Truth
- ❌ Pas de duplication d'état entre pages
- ✅ Toutes les pages lisent/écrivent le même `StateManager`
- Évite désynchronisation

### 6. Zero-Copy sur Teensy
```cpp
// ❌ Mauvais: 32 allocations
std::array<std::string, 32> names = getAllPresetsNames();

// ✅ Bon: 0 allocation, pointeurs directs
const char* names[32];
getAllPresetsNames(names);
```

### 7. Performance UI
- Limiter redraw à 60 FPS (16.6 ms)
- Dirty flag: ne redessiner que si changement
- Double buffering pour animations

### 8. MIDI Low-Latency
- MIDI events contournent UI, vont direct au synth
- Pas de queue UI pour note on/off
- Priorité maximale

---

## 📊 ESTIMATION TEMPS TOTAL

| Phase | Durée | Difficulté |
|-------|-------|------------|
| Foundation | 1-2 sem | ⭐⭐ |
| Input & Events | 1 sem | ⭐⭐ |
| UI Base | 1-2 sem | ⭐⭐⭐ |
| Preset Mgmt | 1 sem | ⭐⭐ |
| MIDI Input | 1 sem | ⭐⭐⭐ |
| Audio Output | 1-2 sem | ⭐⭐⭐⭐ |
| Editing Pages | 2-3 sem | ⭐⭐⭐ |
| Save/Load | 1 sem | ⭐⭐ |
| **TOTAL MVP** | **8-12 sem** | **~3 mois** |
| Graphics V2 | +2-3 sem | ⭐⭐⭐⭐ |

**MVP = Minimum Viable Product** : Tout fonctionne, interface texte simple.

---

## 🎯 PROCHAINES ÉTAPES

1. **Acheter matériel manquant** (encodeurs, MUX, shift registers, optocoupler)
2. **Câblage hardware** sur breadboard
3. **Commencer Sprint 1** : Display + Buttons
4. **Tests incrémentaux** : chaque module testé isolément
5. **Intégration progressive** : ajouter couches une par une

---

## 📚 RESSOURCES

### Composants
- **Écran TFT:** Adafruit ST7789 240×240 ou ILI9341 320×240
- **Encoders:** EC11 avec switch intégré
- **MUX:** CD4051 (8 channels) × 3
- **Shift Reg:** 74HC165 (PISO) × 2
- **Optocoupler:** 6N138 pour MIDI
- **DAC:** PCM5102 (I2S) ou Teensy Audio Board

### Bibliothèques
- `Adafruit_GFX` + `Adafruit_ST7789` / `ILI9341_t3n`
- `Teensy Audio Library` (Paul Stoffregen)
- `MIDI Library` (optionnel, on peut parser à la main)

---

## ✅ VALIDATION ARCHITECTURE

Cette architecture a été conçue pour :
- ✅ **Évolutivité** : Passer de texte à graphique sans refonte
- ✅ **Maintenabilité** : 1 bug = 1 fichier à corriger
- ✅ **Performance** : Zero-copy, event-driven, 60 FPS
- ✅ **Testabilité** : Chaque couche mockable
- ✅ **Intégration** : S'appuie sur `core/` existant sans modification

**Cette architecture est prête pour la production Teensy !** 🚀

---

*Document créé le 12 janvier 2026*  
*Dernière mise à jour : 12 janvier 2026*
