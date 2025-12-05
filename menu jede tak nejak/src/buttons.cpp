#include "buttons.h"
#include <Arduino.h>

// Debounce
void IRAM_ATTR handleLeftPress() {
    static unsigned long lastPress = 0;
    if (millis() - lastPress < 200) return;
    lastPress = millis();

    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    needsRedraw = true;
    
    if(actionInProgress) {
        actionInProgress = false; 
        needsRedraw = true; 
        actionInProgress = true; // Okamžitý restart akce
    }
    // Serial print v ISR není ideální, ale pro debug nutný:
    // Serial.println("LEFT"); 
}

void IRAM_ATTR handleCenterPress() {
    static unsigned long lastPress = 0;
    if (millis() - lastPress < 200) return;
    lastPress = millis();

    if(!actionInProgress) actionInProgress = true;
    else {
        actionInProgress = false; 
        needsRedraw = true; 
    }
}

void IRAM_ATTR handleRightPress() {
    static unsigned long lastPress = 0;
    if (millis() - lastPress < 200) return;
    lastPress = millis();

    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    needsRedraw = true;

    if(actionInProgress) {
        actionInProgress = false; 
        needsRedraw = true;
        actionInProgress = true; 
    }
}

void initButtons() {
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_CENTER, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_LEFT), handleLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CENTER), handleCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), handleRightPress, FALLING);

    Serial.println("✅ Tlačítka inicializována (Piny: 25, 32, 33)");
    Serial.println("ℹ️ Ujisti se, že tlačítka spínají proti GND!");
}
