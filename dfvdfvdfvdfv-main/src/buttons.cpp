#include "buttons.h"
#include <Arduino.h>

// Globální proměnná pro debounce
volatile unsigned long lastPress = 0; 
const unsigned long DEBOUNCE_DELAY = 250; 

// === LEVÉ TLAČÍTKO: POSUN VLEVO ===
void IRAM_ATTR handleLeftPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    // Pokud běží akce, tlačítko ignorujeme
    if (actionInProgress) return; 

    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    needsRedraw = true;
}

// === STŘEDNÍ TLAČÍTKO: SPUŠTĚNÍ AKCE ===
void IRAM_ATTR handleCenterPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    // Tady jen řekneme "ANO, CHCI AKCI".
    // Samotná funkce akce() se spustí až v main loopu.
    if(!actionInProgress) {
        actionInProgress = true; 
    } 
    // Pokud už akce běží, můžeme ji tímto přerušit (volitelné)
    else {
        actionInProgress = false; 
        needsRedraw = true;
    }
}

// === PRAVÉ TLAČÍTKO: POSUN VPRAVO ===
void IRAM_ATTR handleRightPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    if (actionInProgress) return;

    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    needsRedraw = true;
}

void initButtons() {
    pinMode(PIN_LEFT, INPUT_PULLUP);
    pinMode(PIN_CENTER, INPUT_PULLUP);
    pinMode(PIN_RIGHT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_LEFT), handleLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CENTER), handleCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), handleRightPress, FALLING);

    Serial.println("✅ Tlačítka inicializována");
}