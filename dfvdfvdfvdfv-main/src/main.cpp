#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <EasyBuzzer.h> // <--- KNIHOVNA PRO ZVUK

TFT_eSPI tft = TFT_eSPI();
Preferences prefs;

volatile bool actionInProgress = false;
volatile int selectedAction = 0;
volatile bool needsRedraw = false;

// Statistiky
int hunger = 100;
int sleepiness = 100;
int hygiene = 100;
int health = 100;
int happiness = 100; 
bool sick = false; 

// Časování
unsigned long lastDecay = 0;
unsigned long decayInterval = 120000; // 2 minuty

unsigned long lastIllCheck = 0;
unsigned long nextIllTime = 900000; // 15 minut

const char* actionNames[] = {"EAT", "SLEEP", "BATH", "PLAY", "HEAL", "STATUS"};
const char* actionShort[] = {"E", "S", "B", "P", "H", "I"};

#include "buttons.h" 
#include "display.h"
#include "sound.h" // (Necháme náš hlavičkový soubor, ale přepíšeme sound.cpp)
#include "pet.h"

void akce() {
    Serial.printf("\n▶️ Provádím: %s\n", actionNames[selectedAction]);
    
    // Zvuk se spustí uvnitř executeAction
    executeAction(selectedAction);
    
    saveState();
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp"); 
    actionInProgress = false; 
    drawMenu(); 
    Serial.println("⏹️ Hotovo\n");
}

void setup() {
    Serial.begin(115200);
    delay(500);

    initDisplay();
    showLoading();
    
    // === NASTAVENÍ BZUČÁKU PŘES KNIHOVNU ===
    EasyBuzzer.setPin(27); // <--- Tady nastavujeme Pin 27
    EasyBuzzer.stopBeep(); // Pro jistotu hned na začátku ticho

    if (!LittleFS.begin(true)) {
        showError("FS ERROR!");
        while (1);
    }

    initButtons(); 
    loadState(); 
    randomSeed(analogRead(0));

    drawBackground();
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp");
    drawMenu(); 
    
    Serial.println("✅ Setup hotov");
}

void loop() {
    // === TOHLE JE DŮLEŽITÉ ===
    EasyBuzzer.update(); // <--- Tohle musí běžet v loopu, aby to pípalo
    // ==========================

    updateDecay();
    checkIllness();

    if (needsRedraw) {
        drawMenu(); 
        needsRedraw = false;
    }

    if (actionInProgress) {
         akce(); 
    }

    delay(10);
}