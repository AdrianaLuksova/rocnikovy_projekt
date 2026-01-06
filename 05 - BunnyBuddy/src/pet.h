#ifndef PET_H
#define PET_H

#include <Arduino.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include "display.h"
#include "sound.h"


volatile int selectedAction = 0;   // Která ikona je vybraná
volatile bool needsRedraw = true;  // Jestli se má překreslit menu
volatile bool actionInProgress = false; // Jestli se něco děje

int hunger = 100;
int sleepiness = 100;
int hygiene = 100;
int health = 100;
int happiness = 100;
bool sick = false;

unsigned long lastDecay = 0;
unsigned long decayInterval = 60000; 
unsigned long lastIllCheck = 0;
unsigned long nextIllTime = 900000;

Preferences prefs;

extern TFT_eSPI tft; 

// pomocné funkce pro uložení a načtení stavu
void saveState() {
    // otevře složku v paměti s název "pet-state"
    prefs.begin("pet-state", false);
    prefs.putInt("hunger", hunger);
    prefs.putInt("sleep", sleepiness);
    prefs.putInt("hyg", hygiene); 
    prefs.putInt("health", health);
    prefs.putInt("happy", happiness);
    prefs.putBool("sick", sick);
    prefs.end();
}

void loadState() {
    // načte ulozený stav při startu hry
    prefs.begin("pet-state", true);
    // Pokud hodnota neexistuje (nová hra), použije se výchozí hodnota 100
    hunger = prefs.getInt("hunger", 100);
    sleepiness = prefs.getInt("sleep", 100);
    hygiene = prefs.getInt("hyg", 100);
    health = prefs.getInt("health", 100);
    happiness = prefs.getInt("happy", 100);
    sick = prefs.getBool("sick", false);
    prefs.end();
}

// safe delay - zpoždění, které lze přerušit (pokud uživatel stiskne tlačítko pro jinou akci)
bool safeDelay(int ms) {
    unsigned long start = millis();
    // pokud uživatel stiskne tlačítko, přerušíme zpoždění
    while (millis() - start < ms) {
        if (!actionInProgress) {
            playCancel(); // piiip zrušení
            return false;
        }
        delay(10);
    }
    return true;
}

// funkce pro postupné snižování stavů
// beží ve smyčce loop(), každou minutu se sníží hodnoty
void updateDecay() {
    if (millis() - lastDecay > decayInterval) {
        lastDecay = millis();
        bool critical = false; 
        // snížení hodnot
        if (hunger > 0) hunger -= 1;     
        if (sleepiness > 0) sleepiness -= 1; 
        if (hygiene > 0) hygiene -= 1;
        if (happiness > 0) happiness -= 1;
        
        //kontrola jestli je něco na 0
        if (hygiene < 20 || hunger < 20 || happiness < 20) {
            if (health > 0) health -= 1;
        }

        if (hunger == 0 || sleepiness == 0 || hygiene == 0 || happiness == 0 || health == 0) {
            critical = true;
        }

        // pokud je něcco kritické, zahraj alarm
        if (critical) playAlarm();

        //Serial.printf("STATS: H:%d S:%d B:%d Happy:%d HP:%d\n", hunger, sleepiness, hygiene, happiness, health);
        saveState();
    }
}

// funkce pro kontrolu nemoci
// náhodná šance na onemocnění
void checkIllness() {
    if (sick) return; // pokud je už nemocný, nic neděláme
    if (millis() - lastIllCheck > nextIllTime) {
        lastIllCheck = millis();
        int chance = (100 - health);  // zdraví 80 = 20% šance na nemoc
        if (random(0, 100) < chance) {
            sick = true;
            playAlarm();
            drawBunny("/sick1.bmp");
            saveState();
        }
    }
}


// přehrání animace akce (jídlo, koupel, hra)
bool playActionAnimated(const char* frame1, const char* frame2) {
    // pokud je králík nemocný, akci nelze provést(kromě léčby)
    if (sick) {
        playCancel(); 
        return false; 
    }
    //Cyklus animace 3x
    for(int i=0; i<3; i++) {
        drawBunny(frame1);
        if (!safeDelay(300)) return false; 
        drawBunny(frame2);
        if (!safeDelay(300)) return false;
    }
    return true;
}

// vykonání vybrané akce
void executeAction(int action) {
    playStart(); 

    if (!actionInProgress) return;

    switch(action) {
        case 0: // JÍDLO
            if(!playActionAnimated("/eat1.bmp", "/eat2.bmp")) return;
            // Funkce min() zajistí, že nepřekročíme 100%
            hunger = min(100, hunger + 20); 
            health = min(100, health + 2); 
            playSuccess();
            break;

        // Specifická animace pro spánek (více opakování než je králíček vyspaný)
        case 1: // SPÁNEK
            for(int i=0; i<5; i++) {
                drawBunny("/sleep1.bmp");
                if (!safeDelay(500)) return;
                drawBunny("/sleep2.bmp");
                if (!safeDelay(500)) return;
            }
            sleepiness = 100;
            playSuccess();
            break;

        case 2: // KOUPEL
            if(!playActionAnimated("/bath1.bmp", "/bath2.bmp")) return;
            hygiene = 100;
            playSuccess();
            break;
        
        // Hraní stojí energii a jídlo, ale přidává štěstí
        case 3: // HRA
            if(!playActionAnimated("/game1.bmp", "/game2.bmp")) return;
            hunger = max(0, hunger - 5);      
            sleepiness = max(0, sleepiness - 5);
            happiness = min(100, happiness + 30);
            health = min(100, health + 5);
            playSuccess();
            break;

        case 4: // LÉČBA
            if (sick) {
                playTone(1000, 100); if (!safeDelay(100)) return;
                playTone(1500, 100); if (!safeDelay(100)) return;
                playTone(2000, 200);

                for(int i=0; i<3; i++) {
                     drawBunny("/heal1.bmp");
                     if (!safeDelay(300)) return;
                     drawBunny("/heal2.bmp");
                     if (!safeDelay(300)) return;
                }
                sick = false;  // uzdraveno
                health = 100;  
                lastIllCheck = millis(); // reset časovač nemoci
                playSuccess();
            } else {
                playCancel(); 
            }
            break;

        case 5: // STATUS 
            {
                uint16_t pinkBG = tft.color565(255, 182, 193);
                tft.fillScreen(pinkBG);
                tft.setTextColor(TFT_BLACK, pinkBG);
                tft.setTextSize(2);

                // Výpis textových hodnot
                tft.setCursor(10, 20); tft.printf("HLAD:   %d%%\n", hunger);
                tft.setCursor(10, 45); tft.printf("SPANEK: %d%%\n", sleepiness);
                tft.setCursor(10, 70); tft.printf("HYGIENA:%d%%\n", hygiene);
                tft.setCursor(10, 95); tft.printf("HRANI:  %d%%\n", happiness);
                // Výpis celkového stavu (Zdravý/Nemocný)
                tft.setCursor(10, 120);
                if(sick) {
                    tft.setTextColor(TFT_RED, pinkBG);
                    tft.print("STAV:   NEMOC!");
                } else {
                    tft.setTextColor(TFT_DARKGREEN, pinkBG);
                    tft.print("STAV:   ZDRAVY");
                }

                safeDelay(6000); 
            }
            break;
    }
}

#endif
