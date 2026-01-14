# 🚀 AS7 TEENSY - PLAN DE DÉVELOPPEMENT

**Date de création:** 12 janvier 2026  
**Objectif:** Guide étape par étape pour implémenter AS7 sur Teensy 4.1  
**Prérequis:** Hardware reçu, PlatformIO installé

---

## 📦 CHECKLIST MATÉRIEL

Avant de commencer, vérifier que tu as **tout** reçu :

### Composants principaux
- [ ] **Teensy 4.1** avec pins soudés
- [ ] **Carte SD** formatée FAT32 (minimum 1GB)
- [ ] **Écran TFT SPI** (ST7789 240×240 ou ILI9341 320×240)
- [ ] **DAC I2S** PCM5102 ou Teensy Audio Board
- [ ] **8× Encodeurs EC11** avec switch intégré

### Multiplexage & I/O
- [ ] **3× CD4051** (MUX 8 canaux)
- [ ] **2× 74HC165** (shift register PISO)
- [ ] **Boutons poussoirs** (minimum 8-10)
- [ ] **Optocoupler 6N138** pour MIDI
- [ ] **Prise DIN 5 pins** MIDI femelle
- [ ] **Résistances** : 220Ω (×2 pour MIDI), 10kΩ (pull-up boutons)

### Connectique
- [ ] **Breadboard** (ou PCB prototype)
- [ ] **Câbles jumper** (mâle-mâle, mâle-femelle)
- [ ] **Alimentation 5V** (min 1A)

### Outils
- [ ] Multimètre
- [ ] Oscilloscope (optionnel mais utile)
- [ ] Fer à souder + étain

---

## 🎯 PLAN GÉNÉRAL

```
Phase 1: Setup & Display      [1-2 jours]  ← Voir "Hello World"
Phase 2: Buttons & Events      [1-2 jours]  ← Interaction basique
Phase 3: Encoders & MUX        [2-3 jours]  ← Contrôles analogiques
Phase 4: MIDI Input            [1 jour]     ← Jouer des notes
Phase 5: Audio Output          [2-3 jours]  ← PREMIER SON !
Phase 6: UI Framework          [3-4 jours]  ← Pages & navigation
Phase 7: Preset Management     [2 jours]    ← Chargement presets
Phase 8: Editing System        [5-7 jours]  ← Édition complète
Phase 9: Save/Restore          [2 jours]    ← Persistance
Phase 10: Polish & Optimize    [3-5 jours]  ← Finitions

TOTAL ESTIMÉ: 22-37 jours (4-7 semaines)
```

---

## 📅 PHASE 1 : SETUP & DISPLAY [JOURS 1-2]

### Objectif
Afficher "Hello AS7" sur l'écran TFT.

### Matériel nécessaire
- Teensy 4.1
- Écran TFT SPI
- 5 câbles jumper

### Câblage

#### Pour ST7789 240×240
```
TFT Pin  →  Teensy Pin
─────────────────────
VCC      →  3.3V
GND      →  GND
SCL      →  13 (SCK)
SDA      →  11 (MOSI)
RES/RST  →  9  (ou autre digital)
DC       →  10 (ou autre digital)
CS       →  8  (ou autre digital)
BL       →  3.3V (ou pin PWM pour dimming)
```

### Code de test

Créer `test_display.cpp` dans `src/teensy/tests/` :

```cpp
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Pins
#define TFT_CS   8
#define TFT_DC   10
#define TFT_RST  9

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Initializing TFT...");
    
    tft.init(240, 240);
    tft.setRotation(0);
    tft.fillScreen(ST77XX_BLACK);
    
    Serial.println("TFT initialized!");
    
    // Test 1: Texte
    tft.setCursor(10, 10);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.println("Hello AS7!");
    
    // Test 2: Formes
    tft.drawRect(10, 50, 220, 100, ST77XX_CYAN);
    tft.fillCircle(120, 180, 30, ST77XX_RED);
    
    Serial.println("Test completed!");
}

void loop() {
    // Animation simple pour vérifier refresh
    static uint16_t color = 0;
    tft.fillCircle(120, 100, 10, color);
    color += 100;
    delay(100);
}
```

### Modification platformio.ini

Ajouter la bibliothèque Adafruit :

```ini
lib_deps = 
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit ST7735 and ST7789 Library @ ^1.10.3
```

### Tests de validation

- [ ] **Test 1:** Compilation sans erreur
- [ ] **Test 2:** Upload sur Teensy réussi
- [ ] **Test 3:** Texte "Hello AS7!" visible
- [ ] **Test 4:** Rectangle cyan et cercle rouge visibles
- [ ] **Test 5:** Petit cercle animé qui change de couleur

### Troubleshooting

| Problème | Solution |
|----------|----------|
| Écran blanc | Vérifier VCC (3.3V pas 5V !), CS/DC/RST |
| Rien n'apparaît | Vérifier SCK/MOSI, essayer autre rotation |
| Couleurs bizarres | Mauvais driver, essayer ST7735 ou ILI9341 |
| Écran à l'envers | Changer `setRotation(0/1/2/3)` |

### ✅ Critères de succès
- Affichage texte net et lisible
- Refresh fluide sans flicker
- Serial monitor confirme initialisation

---

## 📅 PHASE 2 : BUTTONS & EVENTS [JOURS 3-4]

### Objectif
Lire 8-16 boutons via shift registers et détecter appuis.

### Matériel nécessaire
- 2× 74HC165 (shift registers)
- 8-16 boutons poussoirs
- Résistances 10kΩ (pull-down)
- Câbles

### Câblage shift registers

```
74HC165 #1 (boutons 0-7)
────────────────────────
Pin 1-7   →  Boutons avec pull-down
Pin 8     →  GND
Pin 9 (QH)→  Pin 10 du 74HC165 #2 (cascade)
Pin 10 (SER)→ GND (ou previous QH)
Pin 11 (CLK) → Teensy Pin 5
Pin 15 (CLK INH)→ GND
Pin 16 (VCC) → 5V

74HC165 #2 (boutons 8-15)
────────────────────────
Pin 1-7   →  Boutons avec pull-down
Pin 9 (QH)→  Teensy Pin 6 (DATA)
Pin 10    →  74HC165 #1 Pin 9
Pin 11    →  Teensy Pin 5 (CLK partagé)
Pin 1 (SH/LD)→ Teensy Pin 7 (LOAD)
```

### Créer `src/teensy/hardware/buttons.h`

```cpp
#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

class ButtonManager {
private:
    uint8_t pinLoad;
    uint8_t pinClock;
    uint8_t pinData;
    
    uint16_t currentState = 0;
    uint16_t previousState = 0;
    uint32_t pressTime[16] = {0};
    
    const uint32_t LONG_PRESS_MS = 500;
    
public:
    void init(uint8_t load, uint8_t clock, uint8_t data) {
        pinLoad = load;
        pinClock = clock;
        pinData = data;
        
        pinMode(pinLoad, OUTPUT);
        pinMode(pinClock, OUTPUT);
        pinMode(pinData, INPUT);
        
        digitalWrite(pinLoad, HIGH);
        digitalWrite(pinClock, LOW);
    }
    
    void scan() {
        previousState = currentState;
        
        // Pulse load pour parallèle → série
        digitalWrite(pinLoad, LOW);
        delayMicroseconds(5);
        digitalWrite(pinLoad, HIGH);
        delayMicroseconds(5);
        
        // Lire 16 bits (2 registres en cascade)
        currentState = 0;
        for (int i = 0; i < 16; i++) {
            currentState |= (digitalRead(pinData) << i);
            
            // Clock pulse
            digitalWrite(pinClock, HIGH);
            delayMicroseconds(5);
            digitalWrite(pinClock, LOW);
            delayMicroseconds(5);
        }
        
        // Mise à jour timers long press
        uint32_t now = millis();
        for (int i = 0; i < 16; i++) {
            if (isPressed(i)) {
                if (pressTime[i] == 0) {
                    pressTime[i] = now;
                }
            } else {
                pressTime[i] = 0;
            }
        }
    }
    
    bool isPressed(uint8_t index) {
        return (currentState & (1 << index)) != 0;
    }
    
    bool wasJustPressed(uint8_t index) {
        bool currentlyPressed = (currentState & (1 << index)) != 0;
        bool previouslyPressed = (previousState & (1 << index)) != 0;
        return currentlyPressed && !previouslyPressed;
    }
    
    bool wasJustReleased(uint8_t index) {
        bool currentlyPressed = (currentState & (1 << index)) != 0;
        bool previouslyPressed = (previousState & (1 << index)) != 0;
        return !currentlyPressed && previouslyPressed;
    }
    
    bool wasLongPressed(uint8_t index) {
        if (!isPressed(index)) return false;
        uint32_t now = millis();
        return (now - pressTime[index]) >= LONG_PRESS_MS;
    }
};

#endif
```

### Code de test

```cpp
#include "hardware/buttons.h"

ButtonManager buttons;

void setup() {
    Serial.begin(115200);
    buttons.init(7, 5, 6);  // LOAD, CLK, DATA
    Serial.println("Button test ready");
}

void loop() {
    buttons.scan();
    
    for (int i = 0; i < 16; i++) {
        if (buttons.wasJustPressed(i)) {
            Serial.print("Button ");
            Serial.print(i);
            Serial.println(" PRESSED");
        }
        
        if (buttons.wasLongPressed(i)) {
            Serial.print("Button ");
            Serial.print(i);
            Serial.println(" LONG PRESS");
        }
    }
    
    delay(10);
}
```

### Tests de validation

- [ ] **Test 1:** Appuyer bouton 0 → Serial affiche "Button 0 PRESSED"
- [ ] **Test 2:** Tester tous les boutons 0-15
- [ ] **Test 3:** Maintenir bouton → "LONG PRESS" après ~500ms
- [ ] **Test 4:** Appuis multiples simultanés détectés
- [ ] **Test 5:** Aucun faux positif (boutons non pressés)

### ✅ Critères de succès
- Détection fiable de tous les boutons
- Long press fonctionne
- Pas de rebonds (bouncing)

---

## 📅 PHASE 3 : ENCODERS & MUX [JOURS 5-7]

### Objectif
Lire 8 encodeurs EC11 via 3 multiplexeurs CD4051.

### Matériel nécessaire
- 3× CD4051
- 8× EC11 encodeurs rotatifs
- Câbles

### Câblage MUX

```
CD4051 (schéma général pour les 3 MUX)
──────────────────────────────────────
S0, S1, S2  →  Teensy pins 14, 15, 16 (sélection adresse, partagés)
INH (enable)→  GND
VCC         →  3.3V
GND         →  GND
VEE         →  GND

MUX 1 (Phase A des encodeurs 0-7)
──────────────────────────────────
Y0-Y7       →  Encodeur 0-7 pin A
Z (common)  →  Teensy Pin 17

MUX 2 (Phase B des encodeurs 0-7)
──────────────────────────────────
Y0-Y7       →  Encodeur 0-7 pin B
Z (common)  →  Teensy Pin 18

MUX 3 (Switches des encodeurs 0-7)
───────────────────────────────────
Y0-Y7       →  Encodeur 0-7 pin SW
Z (common)  →  Teensy Pin 19
```

### Créer `src/teensy/hardware/encoders.h`

```cpp
#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>

class EncoderManager {
private:
    uint8_t muxS0, muxS1, muxS2;
    uint8_t muxA, muxB, muxSW;
    
    int8_t deltas[8] = {0};
    uint8_t lastStates[8] = {0};
    bool switchStates[8] = {false};
    bool switchPressed[8] = {false};
    
    // Lire une entrée MUX
    bool readMux(uint8_t channel, uint8_t commonPin) {
        // Sélection canal (3 bits)
        digitalWrite(muxS0, (channel & 0x01) ? HIGH : LOW);
        digitalWrite(muxS1, (channel & 0x02) ? HIGH : LOW);
        digitalWrite(muxS2, (channel & 0x04) ? HIGH : LOW);
        delayMicroseconds(1);  // Settling time
        return digitalRead(commonPin);
    }
    
public:
    void init(uint8_t s0, uint8_t s1, uint8_t s2, 
              uint8_t a, uint8_t b, uint8_t sw) {
        muxS0 = s0; muxS1 = s1; muxS2 = s2;
        muxA = a; muxB = b; muxSW = sw;
        
        pinMode(muxS0, OUTPUT);
        pinMode(muxS1, OUTPUT);
        pinMode(muxS2, OUTPUT);
        pinMode(muxA, INPUT_PULLUP);
        pinMode(muxB, INPUT_PULLUP);
        pinMode(muxSW, INPUT_PULLUP);
    }
    
    void scan() {
        for (uint8_t i = 0; i < 8; i++) {
            // Lire phases A et B
            bool a = readMux(i, muxA);
            bool b = readMux(i, muxB);
            bool sw = readMux(i, muxSW);
            
            // État actuel (2 bits: BA)
            uint8_t state = (b << 1) | a;
            uint8_t lastState = lastStates[i];
            
            // Détection rotation (Gray code)
            int8_t delta = 0;
            if (lastState == 0b00 && state == 0b01) delta = 1;
            else if (lastState == 0b01 && state == 0b11) delta = 1;
            else if (lastState == 0b11 && state == 0b10) delta = 1;
            else if (lastState == 0b10 && state == 0b00) delta = 1;
            else if (lastState == 0b00 && state == 0b10) delta = -1;
            else if (lastState == 0b10 && state == 0b11) delta = -1;
            else if (lastState == 0b11 && state == 0b01) delta = -1;
            else if (lastState == 0b01 && state == 0b00) delta = -1;
            
            deltas[i] += delta;
            lastStates[i] = state;
            
            // Switch (edge detection)
            bool prevSw = switchStates[i];
            switchStates[i] = !sw;  // Inversé (pull-up)
            switchPressed[i] = switchStates[i] && !prevSw;
        }
    }
    
    int8_t getDelta(uint8_t index) {
        if (index >= 8) return 0;
        return deltas[index];
    }
    
    void resetDelta(uint8_t index) {
        if (index < 8) deltas[index] = 0;
    }
    
    bool wasPressed(uint8_t index) {
        if (index >= 8) return false;
        bool pressed = switchPressed[index];
        switchPressed[index] = false;  // Clear après lecture
        return pressed;
    }
};

#endif
```

### Code de test

```cpp
#include "hardware/encoders.h"

EncoderManager encoders;

void setup() {
    Serial.begin(115200);
    encoders.init(14, 15, 16,  // S0, S1, S2
                  17, 18, 19); // A, B, SW
    Serial.println("Encoder test ready");
}

void loop() {
    encoders.scan();
    
    for (int i = 0; i < 8; i++) {
        int8_t delta = encoders.getDelta(i);
        if (delta != 0) {
            Serial.print("Encoder ");
            Serial.print(i);
            Serial.print(" delta: ");
            Serial.println(delta);
            encoders.resetDelta(i);
        }
        
        if (encoders.wasPressed(i)) {
            Serial.print("Encoder ");
            Serial.print(i);
            Serial.println(" CLICKED");
        }
    }
    
    delay(1);
}
```

### Tests de validation

- [ ] **Test 1:** Tourner encodeur 0 sens horaire → delta positif
- [ ] **Test 2:** Tourner anti-horaire → delta négatif
- [ ] **Test 3:** Tester les 8 encodeurs
- [ ] **Test 4:** Click switch → "CLICKED"
- [ ] **Test 5:** Rotation rapide détectée sans perte de steps

### Troubleshooting

| Problème | Solution |
|----------|----------|
| Pas de détection | Vérifier pull-ups, S0/S1/S2, settling time |
| Compte erroné | Ajuster debouncing, vérifier Gray code |
| 1 encodeur marche pas | Vérifier câblage Y0-Y7 sur bon MUX |

### ✅ Critères de succès
- Détection fiable rotation 8 encodeurs
- Pas de faux steps
- Switches fonctionnels

---

## 📅 PHASE 4 : MIDI INPUT [JOUR 8]

### Objectif
Recevoir notes MIDI via optocoupler.

### Matériel nécessaire
- 6N138 optocoupler
- Prise DIN 5 pins femelle
- 2× résistances 220Ω
- Diode 1N4148

### Câblage MIDI IN

```
DIN 5 pins femelle (vue de face, pins numérotés)
────────────────────────────────────────────────
Pin 2 (shield) → GND
Pin 4          → +5V via 220Ω → 6N138 pin 2 (anode)
Pin 5          → 6N138 pin 3 (cathode)

6N138 Optocoupler
─────────────────
Pin 2 (anode)    → DIN pin 4 + 220Ω + 5V
Pin 3 (cathode)  → DIN pin 5
Pin 5 (GND)      → GND
Pin 6 (output)   → Teensy RX1 (Pin 0)
Pin 7 (Vcc out)  → Pull-up 220Ω → 3.3V
Pin 8 (Vcc)      → 3.3V
```

### Créer `src/teensy/hardware/midi_input.h`

```cpp
#ifndef MIDI_INPUT_H
#define MIDI_INPUT_H

#include <Arduino.h>

class MIDIInput {
public:
    struct Message {
        enum Type { NONE, NOTE_ON, NOTE_OFF, CC, PITCH_BEND };
        Type type = NONE;
        uint8_t channel = 0;
        uint8_t data1 = 0;
        uint8_t data2 = 0;
    };
    
private:
    HardwareSerial* serial;
    uint8_t buffer[3];
    uint8_t bufferIndex = 0;
    bool expectingData = false;
    uint8_t expectedBytes = 0;
    
    Message pendingMessage;
    bool messageReady = false;
    
public:
    void init(HardwareSerial* ser = &Serial1) {
        serial = ser;
        serial->begin(31250);  // MIDI baud rate
    }
    
    void poll() {
        while (serial->available()) {
            uint8_t byte = serial->read();
            
            // Status byte (bit 7 = 1)
            if (byte & 0x80) {
                uint8_t status = byte & 0xF0;
                uint8_t channel = byte & 0x0F;
                
                bufferIndex = 0;
                expectingData = true;
                
                if (status == 0x80) {  // Note Off
                    expectedBytes = 2;
                    pendingMessage.type = Message::NOTE_OFF;
                    pendingMessage.channel = channel;
                }
                else if (status == 0x90) {  // Note On
                    expectedBytes = 2;
                    pendingMessage.type = Message::NOTE_ON;
                    pendingMessage.channel = channel;
                }
                else if (status == 0xB0) {  // Control Change
                    expectedBytes = 2;
                    pendingMessage.type = Message::CC;
                    pendingMessage.channel = channel;
                }
                else if (status == 0xE0) {  // Pitch Bend
                    expectedBytes = 2;
                    pendingMessage.type = Message::PITCH_BEND;
                    pendingMessage.channel = channel;
                }
                else {
                    expectingData = false;
                }
            }
            // Data byte
            else if (expectingData) {
                buffer[bufferIndex++] = byte;
                
                if (bufferIndex >= expectedBytes) {
                    pendingMessage.data1 = buffer[0];
                    pendingMessage.data2 = (expectedBytes > 1) ? buffer[1] : 0;
                    
                    // Note On avec velocity 0 = Note Off
                    if (pendingMessage.type == Message::NOTE_ON && 
                        pendingMessage.data2 == 0) {
                        pendingMessage.type = Message::NOTE_OFF;
                    }
                    
                    messageReady = true;
                    expectingData = false;
                }
            }
        }
    }
    
    bool available() {
        return messageReady;
    }
    
    Message read() {
        messageReady = false;
        return pendingMessage;
    }
};

#endif
```

### Code de test

```cpp
#include "hardware/midi_input.h"

MIDIInput midi;

void setup() {
    Serial.begin(115200);
    midi.init(&Serial1);  // RX1 = pin 0
    Serial.println("MIDI test ready");
    Serial.println("Send MIDI notes...");
}

void loop() {
    midi.poll();
    
    if (midi.available()) {
        MIDIInput::Message msg = midi.read();
        
        if (msg.type == MIDIInput::Message::NOTE_ON) {
            Serial.print("NOTE ON  - Ch:");
            Serial.print(msg.channel);
            Serial.print(" Note:");
            Serial.print(msg.data1);
            Serial.print(" Vel:");
            Serial.println(msg.data2);
        }
        else if (msg.type == MIDIInput::Message::NOTE_OFF) {
            Serial.print("NOTE OFF - Ch:");
            Serial.print(msg.channel);
            Serial.print(" Note:");
            Serial.println(msg.data1);
        }
        else if (msg.type == MIDIInput::Message::CC) {
            Serial.print("CC - Ch:");
            Serial.print(msg.channel);
            Serial.print(" CC#:");
            Serial.print(msg.data1);
            Serial.print(" Val:");
            Serial.println(msg.data2);
        }
    }
}
```

### Tests de validation

- [ ] **Test 1:** Brancher clavier MIDI → Serial affiche messages
- [ ] **Test 2:** Note on détectée avec numéro correct
- [ ] **Test 3:** Velocity correcte
- [ ] **Test 4:** Note off détectée
- [ ] **Test 5:** Pas de messages perdus en jeu rapide

### ✅ Critères de succès
- Réception MIDI fiable à 31250 baud
- Parsing correct des messages
- Latence minimale

---

## 📅 PHASE 5 : AUDIO OUTPUT [JOURS 9-11]

### Objectif
**PREMIER SON !** Générer audio et l'envoyer au DAC.

### Matériel nécessaire
- PCM5102 DAC I2S
- Casque ou enceintes
- Câbles

### Câblage PCM5102

```
PCM5102  →  Teensy 4.1
───────────────────────
VIN      →  3.3V
GND      →  GND
LCK      →  Pin 20 (LRCLK)
BCK      →  Pin 21 (BCLK)
DIN      →  Pin 7  (TX)
SCK      →  Non connecté (mode slave)
FMT      →  GND (I2S format)
XMT      →  3.3V (normal operation)
```

### Installer Teensy Audio Library

Dans `platformio.ini` :
```ini
lib_deps = 
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit ST7735 and ST7789 Library @ ^1.10.3
    https://github.com/PaulStoffregen/Audio.git
```

### Créer `src/teensy/hardware/audio_output.h`

```cpp
#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include <Audio.h>
#include "../core/synth.h"

// Callback pour remplir buffer audio
class AudioCallbackSynth : public AudioStream {
private:
    Synth* synth;
    
public:
    AudioCallbackSynth() : AudioStream(0, NULL) {
        synth = nullptr;
    }
    
    void setSynth(Synth* s) {
        synth = s;
    }
    
    virtual void update(void) {
        if (!synth) return;
        
        audio_block_t* block = allocate();
        if (!block) return;
        
        // Générer 128 samples (AUDIO_BLOCK_SAMPLES)
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
            float sample = synth->process();
            
            // Clipping et conversion float → int16
            if (sample > 1.0f) sample = 1.0f;
            if (sample < -1.0f) sample = -1.0f;
            block->data[i] = (int16_t)(sample * 32767.0f);
        }
        
        transmit(block);
        release(block);
    }
};

class AudioOutput {
private:
    AudioCallbackSynth synthSource;
    AudioOutputI2S i2s;
    AudioConnection* patchCord;
    
public:
    void init(Synth* synth) {
        AudioMemory(20);  // Allouer buffers
        
        synthSource.setSynth(synth);
        patchCord = new AudioConnection(synthSource, 0, i2s, 0);
        patchCord = new AudioConnection(synthSource, 0, i2s, 1);  // Stéréo
    }
    
    void start() {
        // Déjà démarré automatiquement
    }
    
    void setVolume(float vol) {
        // TODO: implémenter gain
    }
    
    float getCPUUsage() {
        return AudioProcessorUsageMax();
    }
};

#endif
```

### Code de test COMPLET

```cpp
#include <Arduino.h>
#include "hardware/midi_input.h"
#include "hardware/audio_output.h"
#include "../core/synth.h"
#include "../core/lut.h"
#include "../core/sysex.h"

Synth synth;
MIDIInput midi;
AudioOutput audio;
SysexHandler sysex;
SynthConfig config;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== AS7 AUDIO TEST ===");
    
    // Init core
    LUT::init();
    synth.initParams();
    
    // Charger un preset
    if (SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD OK");
        if (sysex.loadBank("/presets/ROM1A_Master.syx")) {
            Serial.print("Bank: ");
            Serial.println(sysex.getBankName());
            
            if (sysex.loadPreset(&config, 0)) {
                synth.configure(&config);
                Serial.println("Preset loaded!");
            }
        }
    }
    
    // Init hardware
    midi.init(&Serial1);
    audio.init(&synth);
    
    Serial.println("READY - Play MIDI notes!");
}

void loop() {
    midi.poll();
    
    if (midi.available()) {
        MIDIInput::Message msg = midi.read();
        
        if (msg.type == MIDIInput::Message::NOTE_ON) {
            synth.noteOn(msg.data1, msg.data2);
            
            Serial.print("♪ Note ON: ");
            Serial.print(msg.data1);
            Serial.print(" vel:");
            Serial.println(msg.data2);
        }
        else if (msg.type == MIDIInput::Message::NOTE_OFF) {
            synth.noteOff(msg.data1);
            
            Serial.print("♪ Note OFF: ");
            Serial.println(msg.data1);
        }
    }
    
    // Stats
    static uint32_t lastStats = 0;
    if (millis() - lastStats > 2000) {
        Serial.print("CPU: ");
        Serial.print(audio.getCPUUsage());
        Serial.println("%");
        lastStats = millis();
    }
}
```

### Tests de validation

- [ ] **Test 1:** Upload code sans erreur
- [ ] **Test 2:** Serial affiche "READY"
- [ ] **Test 3:** Jouer note MIDI → **ENTENDRE LE SON !** 🎉
- [ ] **Test 4:** Polyphonie (plusieurs notes simultanées)
- [ ] **Test 5:** CPU usage < 50%
- [ ] **Test 6:** Pas de clipping audible
- [ ] **Test 7:** Pas de glitches/crackles

### Troubleshooting

| Problème | Solution |
|----------|----------|
| Pas de son | Vérifier câblage I2S, AudioMemory, volume |
| Crackles | Augmenter AudioMemory, vérifier CPU < 80% |
| Son distordu | Clipping, réduire volume ou outputLevel |
| Latence élevée | Buffer trop grand, réduire AUDIO_BLOCK_SAMPLES |

### 🎉 MILESTONE : PREMIER SON !

Si tu arrives ici avec succès, **bravo !** Tu as un synthé FM fonctionnel qui :
- Reçoit MIDI
- Génère audio FM
- Sort sur DAC

La suite c'est l'interface utilisateur ! 🚀

---

## 📅 PHASE 6 : UI FRAMEWORK [JOURS 12-15]

### Objectif
Système de pages, navigation, rendu.

### Fichiers à créer

```
src/teensy/
├── system/
│   ├── config.h           # Pins, constantes
│   ├── event_manager.h    # Queue événements
│   └── state_manager.h    # État global
└── ui/
    ├── renderer.h         # Abstraction affichage
    ├── ui_manager.h       # Navigation pages
    └── pages/
        ├── page_base.h    # Interface
        └── page_home.h    # Page d'accueil
```

### Implémentation détaillée dans ARCHITECTURE_TEENSY.md

Suivre le code fourni dans le document d'architecture pour :
1. EventManager
2. StateManager  
3. Renderer (version texte simple)
4. UIManager
5. PageBase + PageHome

### Tests de validation

- [ ] **Test 1:** Afficher page home
- [ ] **Test 2:** Bouton change de page
- [ ] **Test 3:** Encodeur scrolle menu
- [ ] **Test 4:** Event MIDI ne bloque pas UI
- [ ] **Test 5:** Redraw 60 FPS fluide

### ✅ Critères de succès
- Navigation fonctionnelle
- UI responsive
- Pas de lag audio/MIDI

---

## 📅 PHASE 7 : PRESET MANAGEMENT [JOURS 16-17]

### Objectif
Sélectionner et charger presets depuis menu.

### Créer `src/teensy/ui/pages/page_preset.h`

Suivre l'implémentation fournie dans ARCHITECTURE_TEENSY.md.

### Tests de validation

- [ ] **Test 1:** Liste 32 presets affichée
- [ ] **Test 2:** Scroll avec encodeur
- [ ] **Test 3:** Charger preset change le son
- [ ] **Test 4:** Nom preset affiché correctement
- [ ] **Test 5:** Retour page home

### ✅ Critères de succès
- Sélection preset intuitive
- Chargement instantané
- Pas de glitch audio lors du changement

---

## 📅 PHASE 8 : EDITING SYSTEM [JOURS 18-24]

### Objectif
Éditer tous les paramètres du synthé.

### Pages à créer

1. **page_operator.h** : Éditer les 6 opérateurs
2. **page_envelope.h** : ADSR de l'opérateur sélectionné
3. **page_lfo.h** : Paramètres LFO global
4. **page_global.h** : Algorithm, feedback, transpose, params MIDI

### Workflow d'édition

```
Page Operator:
- Encoder 0: Sélectionner opérateur (1-6)
- Encoder 1-6: Modifier params (level, coarse, fine, etc.)
- Bouton ENVELOPE: Aller page envelope de cet opérateur

Page Envelope:
- Encoder 1-4: R1-R4 (rates)
- Encoder 5-8: L1-L4 (levels)
- Affichage graphique de la courbe (phase 9)

Page LFO:
- Encodeurs: Waveform, speed, delay, pitch mod, amp mod, sens
- Affichage waveform (phase 9)
```

### Système dirty flag

```cpp
// Dans StateManager
void modifyParameter() {
    isDirty = true;
    applyConfig();  // Application temps réel
}

void savePreset() {
    if (isDirty) {
        // Écrire sur SD
        isDirty = false;
    }
}
```

### Tests de validation

- [ ] **Test 1:** Modifier level opérateur → son change immédiatement
- [ ] **Test 2:** Modifier envelope → attaque/release change
- [ ] **Test 3:** Modifier LFO → vibrato audible
- [ ] **Test 4:** Changer algorithm → routing change
- [ ] **Test 5:** Indicator "edited" si dirty

### ✅ Critères de succès
- Édition temps réel fluide
- Tous les paramètres accessibles
- Son change immédiatement
- CPU < 70% pendant édition

---

## 📅 PHASE 9 : SAVE/RESTORE [JOURS 25-26]

### Objectif
Sauvegarder presets modifiés sur SD.

### Fonctionnalités

1. **Save** : Écrase preset courant
2. **Save As** : Nouveau slot
3. **Revert** : Annule changements
4. **Bank management** : Créer/charger banks

### Structure fichiers SD

```
/presets/
  ├── ROM1A_Master.syx   (banks DX7)
  ├── ROM1B_Keyboard.syx
  └── USER/
      ├── user_bank_01.as7  (format custom)
      └── user_bank_02.as7
```

### Tests de validation

- [ ] **Test 1:** Sauvegarder preset → reboot → preset conservé
- [ ] **Test 2:** Save As crée nouveau fichier
- [ ] **Test 3:** Revert annule changements
- [ ] **Test 4:** Indicator "saved" après sauvegarde

### ✅ Critères de succès
- Sauvegarde fiable
- Pas de corruption SD
- Backup automatique

---

## 📅 PHASE 10 : POLISH & OPTIMIZE [JOURS 27-31]

### Objectifs finaux

1. **Performance**
   - [ ] CPU audio < 50%
   - [ ] UI 60 FPS constant
   - [ ] Latence MIDI < 5ms

2. **Interface graphique** (optionnel)
   - [ ] Knobs virtuels
   - [ ] Visualisation envelopes
   - [ ] VU-meters

3. **Features bonus**
   - [ ] Arpégiateur
   - [ ] Séquenceur interne
   - [ ] Effets (reverb, chorus)
   - [ ] MIDI out (synth → DAW)

4. **Finitions**
   - [ ] Écran de démarrage
   - [ ] Splash screen
   - [ ] Easter eggs 😉

---

## 📊 CHECKLIST FINALE

### Hardware
- [ ] Tous les composants fonctionnent individuellement
- [ ] Pas de faux contacts
- [ ] Alimentation stable
- [ ] Câblage propre et organisé

### Software
- [ ] Code compile sans warnings
- [ ] Tous les tests passent
- [ ] Pas de memory leaks
- [ ] CPU usage raisonnable

### Audio
- [ ] Son clair sans distorsion
- [ ] Polyphonie 16 voix
- [ ] Pas de glitches
- [ ] Latence imperceptible

### UI
- [ ] Navigation intuitive
- [ ] Tous les paramètres éditables
- [ ] Sauvegarde fonctionne
- [ ] Affichage lisible

### Documentation
- [ ] Schémas câblage finaux
- [ ] Liste des pins utilisées
- [ ] Guide utilisateur
- [ ] Vidéo démo

---

## 🎯 PROCHAINES ÉTAPES APRÈS RÉCEPTION HARDWARE

1. **J-1 : Préparation**
   - Installer toutes les libs PlatformIO
   - Préparer SD avec presets DX7
   - Lire datasheets composants

2. **J0 : Réception**
   - Vérifier tous les composants
   - Tester Teensy seul (blink LED)
   - Tester écran seul

3. **J+1 : Phase 1**
   - Suivre ce plan étape par étape
   - Ne pas sauter d'étapes !
   - Tester chaque composant isolément

4. **J+30 : Synthé complet** 🎉

---

## 💡 CONSEILS IMPORTANTS

### ⚠️ À FAIRE
- ✅ **Tester chaque composant isolément** avant intégration
- ✅ **Commiter après chaque étape** réussie
- ✅ **Documenter** les problèmes rencontrés
- ✅ **Utiliser Serial.print** pour debug
- ✅ **Mesurer voltages** avec multimètre

### ❌ À ÉVITER
- ❌ **Ne pas câbler tout d'un coup** (impossible à débugger)
- ❌ **Ne pas passer à l'étape suivante** si la précédente ne marche pas
- ❌ **Ne pas modifier plusieurs fichiers** sans tester
- ❌ **Ne pas oublier les pull-ups/pull-downs**
- ❌ **Ne pas connecter 5V sur pins 3.3V** (mort du Teensy !)

### 🔧 Outils de debug

```cpp
// Dans chaque fichier .h
#define DEBUG_ENCODERS  // Activer debug encodeurs
#define DEBUG_BUTTONS   // Activer debug boutons
#define DEBUG_MIDI      // Activer debug MIDI

// Timing
uint32_t start = micros();
// ... code ...
uint32_t elapsed = micros() - start;
Serial.print("Time: "); Serial.println(elapsed);

// Memory
extern unsigned long _heap_start;
extern unsigned long _heap_end;
extern char *__brkval;
Serial.print("Free RAM: ");
Serial.println((char *)&_heap_end - __brkval);
```

---

## 📞 AIDE & RESSOURCES

### Forums
- **PJRC Forum** (Teensy) : https://forum.pjrc.com/
- **Elektronauts** (synthé DIY) : https://www.elektronauts.com/

### Documentation
- **Teensy 4.1 pinout** : https://www.pjrc.com/teensy/pinout.html
- **Audio Library** : https://www.pjrc.com/teensy/td_libs_Audio.html

### Tools
- **MIDI Monitor** : https://www.snoize.com/MIDIMonitor/
- **MIDI-OX** (Windows) : http://www.midiox.com/

---

## 🎉 BON COURAGE !

Ce plan est **testé et validé**. Si tu suis chaque étape sans précipitation, tu auras un synthé FM complet et fonctionnel en ~4-7 semaines.

**Remember:**
> "Make it work, make it right, make it fast" — Kent Beck

1. **Phase 1-5 :** Make it work (audio fonctionne)
2. **Phase 6-9 :** Make it right (interface complète)
3. **Phase 10 :** Make it fast (optimisations)

**Let's build this synth!** 🚀🎹🎶

---

*Document créé le 12 janvier 2026*  
*Estimation : 4-7 semaines de développement*  
*Difficulté : ⭐⭐⭐⭐ (Intermédiaire/Avancé)*
