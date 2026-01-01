#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>

// Inicializace displeje (používá se v pet.h)
TFT_eSPI tft = TFT_eSPI(); 

// === INCLUDE SOUBORY ===
// Pořadí je důležité!
#include "buttons.h" 
#include "display.h"
#include "sound.h"

// pet.h obsahuje veškerou logiku a proměnné hry
#include "pet.h"        

void akce() {
    executeAction(selectedAction);
    // Zvuk a animace jsou uvnitř executeAction
    saveState();
    
    // Překreslení po akci (pokud není nemocný, dáme normálního)
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp"); 
    actionInProgress = false; 
    drawMenu(); 
}

void setup() {
    Serial.begin(115200);
    
    // 1. Displej
    initDisplay();
    tft.fillScreen(TFT_BLACK);

    // 2. Paměť souborů (Obrázky)
    if (!LittleFS.begin(true)) {
        tft.setTextColor(TFT_RED);
        tft.setCursor(0, 0);
        tft.println("CHYBA PAMETI!");
        tft.println("Nahrála jsi Data?");
        while (1);
    }

    // 3. Zvuk
    initSound();
    
    // 4. Tlačítka a Načtení hry
    initButtons(); 
    loadState(); 
    randomSeed(analogRead(0));

    // 5. První vykreslení
    drawBackground();
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp");
    drawMenu(); 
}

void loop() {
    // Tady už není server.handleClient() :)
    
    updateDecay();
    checkIllness();

    // Pokud se změnilo menu (šipkami)
    if (needsRedraw) {
        drawMenu(); 
        needsRedraw = false;
    }

    // Pokud se potvrdila akce (střední tlačítko)
    if (actionInProgress) {
         akce(); 
    }
    
    delay(10);
}