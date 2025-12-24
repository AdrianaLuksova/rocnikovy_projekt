#ifndef PET_H
#define PET_H

#include <Arduino.h>
#include <Preferences.h>
#include "display.h"
#include "sound.h" 

extern volatile bool actionInProgress;

extern int hunger, sleepiness, hygiene, health, happiness;
extern bool sick;
extern unsigned long lastDecay, decayInterval;
extern unsigned long lastIllCheck, nextIllTime;
extern Preferences prefs;

void saveState() {
    prefs.begin("pet-state", false);
    prefs.putInt("hunger", hunger);
    prefs.putInt("sleep", sleepiness);
    prefs.putInt("hygiene", hygiene);
    prefs.putInt("health", health);
    prefs.putInt("happy", happiness);
    prefs.putBool("sick", sick);
    prefs.end();
}

void loadState() {
    prefs.begin("pet-state", true);
    hunger = prefs.getInt("hunger", 100);
    sleepiness = prefs.getInt("sleep", 100);
    hygiene = prefs.getInt("hygiene", 100);
    health = prefs.getInt("health", 100);
    happiness = prefs.getInt("happy", 100);
    sick = prefs.getBool("sick", false);
    prefs.end();
}

void updateDecay() {
    if (millis() - lastDecay > decayInterval) {
        lastDecay = millis();
        bool critical = false; 

        if (hunger > 0) hunger -= 1;     
        if (sleepiness > 0) sleepiness -= 1; 
        if (hygiene > 0) hygiene -= 1;
        if (happiness > 0) happiness -= 1;
        
        if (hygiene < 20 || hunger < 20 || happiness < 20) {
            if (health > 0) health -= 1;
        }

        if (hunger == 0 || sleepiness == 0 || hygiene == 0 || happiness == 0 || health == 0) {
            critical = true;
        }

        if (critical) playAlarm();

        Serial.printf("📉 STATS: H:%d S:%d B:%d Happy:%d Imm:%d\n", hunger, sleepiness, hygiene, happiness, health);
        saveState();
    }
}

void checkIllness() {
    if (sick) return;
    if (millis() - lastIllCheck > nextIllTime) {
        lastIllCheck = millis();
        int chance = (100 - health); 
        if (random(0, 100) < chance) {
            sick = true;
            playAlarm();
            drawBunny("/sick1.bmp");
            saveState();
        }
    }
}

bool safeDelay(int ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        
        EasyBuzzer.update(); // <--- PŘIDAT TOTO (aby to pípalo i během čekání)

        if (!actionInProgress) {
            playCancel();
            return false;
        }
        delay(10);
    }
    return true;
}

bool playActionAnimated(const char* frame1, const char* frame2) {
    if (sick) {
        playCancel(); 
        return false; 
    }
    for(int i=0; i<3; i++) {
        drawBunny(frame1);
        if (!safeDelay(300)) return false; 
        drawBunny(frame2);
        if (!safeDelay(300)) return false;
    }
    return true;
}

// === TADY JE TA HLAVNÍ ZMĚNA VE STATUSU ===
void executeAction(int action) {
    playStart(); 

    if (!actionInProgress) return;

    switch(action) {
        case 0: // JÍDLO
            if(!playActionAnimated("/eat1.bmp", "/eat2.bmp")) return;
            hunger = min(100, hunger + 20); 
            health = min(100, health + 2); 
            break;

        case 1: // SPÁNEK
            for(int i=0; i<5; i++) {
                drawBunny("/sleep1.bmp");
                if (!safeDelay(500)) return;
                drawBunny("/sleep2.bmp");
                if (!safeDelay(500)) return;
            }
            sleepiness = 100;
            break;

        case 2: // KOUPEL
            if(!playActionAnimated("/bath1.bmp", "/bath2.bmp")) return;
            hygiene = 100;
            break;

        case 3: // HRA
            if(!playActionAnimated("/game1.bmp", "/game2.bmp")) return;
            hunger = max(0, hunger - 5);      
            sleepiness = max(0, sleepiness - 5);
            happiness = min(100, happiness + 30);
            health = min(100, health + 5);
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
                sick = false;  
                health = 100;  
                lastIllCheck = millis(); 
            }
            break;

        case 5: // === STATUS (PŘEDĚLANÝ VZHLED) ===
            {
                // 1. Růžové pozadí (R=255, G=182, B=193 - LightPink)
                uint16_t pinkBG = tft.color565(255, 182, 193);
                tft.fillScreen(pinkBG);
                
                // 2. Černý text (lépe čitelný na růžové)
                tft.setTextColor(TFT_BLACK, pinkBG);
                tft.setTextSize(2);

                // Výpis statistik
                tft.setCursor(10, 20); tft.printf("HLAD:   %d%%\n", hunger);
                tft.setCursor(10, 45); tft.printf("SPANEK: %d%%\n", sleepiness);
                tft.setCursor(10, 70); tft.printf("HYGIENA:%d%%\n", hygiene);
                tft.setCursor(10, 95); tft.printf("HRANI:  %d%%\n", happiness);
                
                tft.setCursor(10, 120);
                if(sick) {
                    tft.setTextColor(TFT_RED, pinkBG); // Červený text pro nemoc
                    tft.print("STAV:   NEMOC!");
                } else {
                    tft.setTextColor(TFT_DARKGREEN, pinkBG); // Zelený pro OK
                    tft.print("STAV:   OK");
                }

                // === 3. BATERIE (Pravý horní roh) ===
                // Pin 14 měří napětí (dělič 1/2)
                uint16_t v = analogRead(14);
                float voltage = ((float)v / 4095.0) * 3.3 * 2.0; // Přepočet na Volty
                
                // Přepočet na procenta (3.0V = 0%, 4.2V = 100%)
                int batPct = map((long)(voltage * 100), 300, 420, 0, 100);
                if (batPct > 100) batPct = 100;
                if (batPct < 0) batPct = 0;

                // Kreslení baterky
                int batX = 190;
                int batY = 5;
                // Obrys
                tft.drawRect(batX, batY, 40, 15, TFT_BLACK);
                tft.drawRect(batX + 40, batY + 4, 3, 7, TFT_BLACK); // "Čudlík" baterie
                
                // Výplň (Zelená > 20%, Červená < 20%)
                uint16_t batColor = (batPct > 20) ? TFT_GREEN : TFT_RED;
                // Šířka výplně podle procent (max 38px)
                int fillW = (batPct * 38) / 100;
                tft.fillRect(batX + 1, batY + 1, fillW, 13, batColor);

                // Text procent pod baterkou (malým písmem)
                tft.setTextSize(1);
                tft.setTextColor(TFT_BLACK, pinkBG);
                tft.setCursor(batX, batY + 18);
                tft.printf("%d%%", batPct);
            }
            
            // Čekání na statusu s možností přerušení (déle - 6 sekund)
            safeDelay(6000); 
            break;
    }
}

#endif