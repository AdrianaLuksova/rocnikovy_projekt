#include "buttons.h"
#include <Arduino.h>

volatile unsigned long lastPress = 0; 
const unsigned long DEBOUNCE_DELAY = 200;

void IRAM_ATTR handleLeftPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    needsRedraw = true;
    
    if(actionInProgress) {
        actionInProgress = false; 
        needsRedraw = true; 
        actionInProgress = true;
    }
}

void IRAM_ATTR handleCenterPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    if(!actionInProgress) actionInProgress = true;
    else {
        actionInProgress = false; 
        needsRedraw = true; 
    }
}

void IRAM_ATTR handleRightPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
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
}
