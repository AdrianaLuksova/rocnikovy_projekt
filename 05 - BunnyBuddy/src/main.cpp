#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>

// Inicializace displeje 
TFT_eSPI tft = TFT_eSPI(); 

// Připojení našich hlavičkových souborů
#include "buttons.h" 
#include "display.h"
#include "sound.h"
#include "pet.h"        

// Funkce pro provedení akce a překreslení
void akce() {
    executeAction(selectedAction); // Vykoná logiku (pet.h)
    saveState();                   // Uloží hru
    
    // Překreslení králíka po akci (pokud není nemocný, vrátí základní výraz)
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp"); 
    actionInProgress = false;      // Uvolníme "zámek" akce
    drawMenu();                    // Překreslíme menu (rámeček)
}

void setup() {
    Serial.begin(115200);
    
    // 1. Inicializace Displeje
    initDisplay();
    tft.fillScreen(TFT_BLACK);

    // 2. Inicializace Souborového systému (Obrázky)
    if (!LittleFS.begin(true)) {
        tft.setTextColor(TFT_RED);
        tft.setCursor(0, 0);
        tft.println("CHYBA PAMETI!");
        tft.println("Nahrala jsi Data?");
        while (1); // Zastaví program
    }

    // 3. Inicializace Zvuku (EasyBuzzer)
    initSound();
    
    // 4. Tlačítka a Načtení uložené hry
    initButtons(); 
    loadState(); 
    
    // Náhodné číslo podle šumu na nezapojeném pinu
    randomSeed(analogRead(0));

    // 5. První vykreslení po startu
    drawBackground(); // Pokud máš tuto funkci v display.h
    drawBunny(sick ? "/sick1.bmp" : "/base1.bmp");
    drawMenu(); 
}

void loop() {
    // 1. DŮLEŽITÉ: Aktualizace zvuku (musí běžet v každém cyklu!)
    updateSound();

    // 2. Herní logika (časování hladu, nemoci)
    updateDecay();
    checkIllness();

    // 3. Překreslení menu při pohybu šipkami
    if (needsRedraw) {
        drawMenu(); 
        needsRedraw = false;
    }

    // 4. Spuštění akce při potvrzení středním tlačítkem
    if (actionInProgress) {
         akce(); 
    }
    
    delay(10); // Krátká pauza pro ulehčení procesoru
}