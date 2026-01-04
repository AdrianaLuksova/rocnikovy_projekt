#ifndef PET_H
#define PET_H

#include <Preferences.h>
#include "display.h"

extern Preferences prefs;
extern TFT_eSPI tft;
extern volatile bool needsRedraw;

// === Stavy mazlíčka ===
extern int hunger;
extern int sleepiness;
extern int hygiene;
extern int health;
extern bool sick;

extern unsigned long lastDecay;
extern const unsigned long decayInterval;

extern unsigned long lastIllCheck;
extern unsigned long nextIllTime;

// === Názvy akcí ===
extern const char* actionNames[];

// === Uložení stavu ===
void saveState() {
  prefs.begin("pet", false);
  prefs.putInt("hunger", hunger);
  prefs.putInt("sleepiness", sleepiness);
  prefs.putInt("hygiene", hygiene);
  prefs.putInt("health", health);
  prefs.putBool("sick", sick);
  prefs.end();
}

// === Načtení stavu ===
void loadState() {
  prefs.begin("pet", false);
  hunger = prefs.getInt("hunger", 100);
  sleepiness = prefs.getInt("sleepiness", 100);
  hygiene = prefs.getInt("hygiene", 100);
  health = prefs.getInt("health", 100);
  sick = prefs.getBool("sick", false);
  prefs.end();
  
  Serial.printf("📂 Loaded: H=%d S=%d Hy=%d He=%d Sick=%d\n", 
                hunger, sleepiness, hygiene, health, sick);
}

// === Decay stavů ===
void updateDecay() {
  if (millis() - lastDecay > decayInterval) {
    lastDecay = millis();
    hunger = max(0, hunger - 1);
    sleepiness = max(0, sleepiness - 1);
    hygiene = max(0, hygiene - 1);
    health = max(0, health - (sick ? 2 : 0));
    saveState();
    Serial.printf("📉 Decay: H=%d S=%d Hy=%d He=%d\n", 
                  hunger, sleepiness, hygiene, health);
  }
}

// === Kontrola nemoci ===
void checkIllness() {
  if (!sick && millis() - lastIllCheck > nextIllTime) {
    lastIllCheck = millis();
    
    int illChance = 7;
    if (hunger < 30) illChance -= 1;
    if (sleepiness < 30) illChance -= 1;
    if (hygiene < 30) illChance -= 2;
    
    if (random(0, 10) > illChance) {
      sick = true;
      Serial.println("🤒 ONEMOCNĚL!");
    }
    nextIllTime = random(60000, 180000);
  }
}

// === Animace bez blokování ===
void playActionNonBlocking(const char *bmp1, const char *bmp2) {
  for (int i = 0; i < 3; i++) {
    tft.fillRect(0, 0, 240, 110, TFT_BLACK);
    drawBmp(bmp1, 0, 0);
    
    for (int j = 0; j < 30; j++) {
      if (needsRedraw) return;
      delay(10);
    }
    
    tft.fillRect(0, 0, 240, 110, TFT_BLACK);
    drawBmp(bmp2, 0, 0);
    
    for (int j = 0; j < 30; j++) {
      if (needsRedraw) return;
      delay(10);
    }
  }
}

// === Provedení akce ===
void executeAction(int action) {
  Serial.printf("🎬 Akce: %s\n", actionNames[action]);
  
  switch (action) {
    case 0: // JÍDLO
      playActionNonBlocking("/eat1.bmp", "/eat2.bmp"); 
      hunger = min(100, hunger + 20);
      Serial.printf("🍖 Hlad: %d\n", hunger);
      break;
      
    case 1: // SPÁNEK
      playActionNonBlocking("/sleep1.bmp", "/sleep2.bmp"); 
      sleepiness = min(100, sleepiness + 20);
      Serial.printf("😴 Spánek: %d\n", sleepiness);
      break;
      
    case 2: // KOUPEL
      playActionNonBlocking("/bath1.bmp", "/bath2.bmp"); 
      hygiene = min(100, hygiene + 20);
      Serial.printf("🧼 Hygiena: %d\n", hygiene);
      break;
      
    case 3: // HRA
      playActionNonBlocking("/play1.bmp", "/play2.bmp"); 
      health = min(100, health + 10);
      hunger = max(0, hunger - 5);
      Serial.println("🎮 Hra: +10 zdraví, -5 hlad");
      break;
      
    case 4: // LÉČBA
      if (sick) {
        playActionNonBlocking("/heal1.bmp", "/heal2.bmp");
        sick = false;
        health = min(100, health + 30);
        Serial.println("💊 VYLÉČENO!");
      } else {
        playActionNonBlocking("/heal1.bmp", "/heal2.bmp");
        Serial.println("💊 Není nemocný");
      }
      break;
      
    case 5: // STAV
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.setCursor(10, 10);
      tft.println("=== STAV ===");
      
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(10, 40);
      tft.printf("Hlad:    %d%%\n", hunger);
      tft.printf("Spanek:  %d%%\n", sleepiness);
      tft.printf("Hygiena: %d%%\n", hygiene);
      tft.printf("Zdravi:  %d%%\n", health);
      
      if (sick) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(10, 100);
        tft.println("NEMOCNY!");
      }
      
      unsigned long startWait = millis();
      while (millis() - startWait < 3000) {
        if (needsRedraw) break;
        delay(10);
      }
      break;
  }

  saveState();
  drawIdle();
}

#endif