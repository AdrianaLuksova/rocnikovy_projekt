#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// === Piny tlačítek ===
// Bezpečné piny pro LILYGO T-Display
const int BTN_LEFT = 25;     // Pin 1 na deštičce (SDA)
const int BTN_OK = 33;       // Pin 2 na deštičce (SCL)
const int BTN_RIGHT = 32;    // Pin 3 na deštičce

// === Globální proměnné pro interrupty ===
extern volatile bool actionInProgress;
extern volatile int selectedAction;
extern volatile bool needsRedraw;
extern const int ACTION_COUNT;

// === Interrupt handlery ===
void IRAM_ATTR onLeft() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 200) {
    lastPress = now;
    if (!actionInProgress) {
      selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
      needsRedraw = true;
      Serial.println("⬅️ LEFT pressed");
    }
  }
}

void IRAM_ATTR onRight() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 200) {
    lastPress = now;
    if (!actionInProgress) {
      selectedAction = (selectedAction + 1) % ACTION_COUNT;
      needsRedraw = true;
      Serial.println("➡️ RIGHT pressed");
    }
  }
}

void IRAM_ATTR onOK() {
  static unsigned long lastPress = 0;
  unsigned long now = millis();
  if (now - lastPress > 300) {
    lastPress = now;
    if (!actionInProgress) {
      actionInProgress = true;
      Serial.println("✅ OK pressed");
    }
  }
}

// === Inicializace tlačítek ===
void initButtons() {
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  
  delay(50);
  
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), onLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_OK), onOK, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), onRight, FALLING);
  
  Serial.println("✅ Tlačítka inicializována");
  Serial.printf("   GPIO %d = LEFT\n", BTN_LEFT);
  Serial.printf("   GPIO %d = OK\n", BTN_OK);
  Serial.printf("   GPIO %d = RIGHT\n", BTN_RIGHT);
  Serial.println("\n📍 Zapojení:");
  Serial.println("   Deštička pin 1 → GPIO 21");
  Serial.println("   Deštička pin 2 → GPIO 22");
  Serial.println("   Deštička pin 3 → GPIO 17");
  Serial.println("   Deštička GND (-) → GND");
  Serial.printf("\n   Status: L=%d O=%d R=%d (1=nestisknuto)\n", 
                digitalRead(BTN_LEFT), 
                digitalRead(BTN_OK),
                digitalRead(BTN_RIGHT));
}

// === Debug tlačítek ===
void debugButtons() {
  Serial.printf("🔘 L=%d O=%d R=%d | selectedAction=%d | actionInProgress=%d\n", 
                digitalRead(BTN_LEFT), 
                digitalRead(BTN_OK),
                digitalRead(BTN_RIGHT),
                selectedAction,
                actionInProgress);
}

#endif