#include "buttons.h"
#include <Arduino.h>

// Globální proměnná pro debounce
// volatile říká kompilátoru, že se hodnota může změnit kdykoliv
volatile unsigned long lastPress = 0; 
const unsigned long DEBOUNCE_DELAY = 250; 

// leve tlačítko
// IRAM_ATTR je nutné pro ESP32, aby funkce běžela v rychlé paměti RAM
void IRAM_ATTR handleLeftPress() {
    // 1. Kontrola času: Pokud uplynulo málo času od minula, končíme
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    // 2. Blokace: Pokud králík zrovna provádí akci, tlačítko ignorujeme
    if (actionInProgress) return; 

    // 3. Matematika menu: Posun indexu doleva.
    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;

    // 4. Nastavíme, že se má překreslit menu
    needsRedraw = true;
}

// strední tlačítko
void IRAM_ATTR handleCenterPress() {
    if (millis() - lastPress < DEBOUNCE_DELAY) return;
    lastPress = millis();

    if(!actionInProgress) {
        actionInProgress = true; 
    } 
    else {
        actionInProgress = false; 
        needsRedraw = true;
    }
}

// pravé tlačítko
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

    // Připojení přerušení k tlačítkům
    attachInterrupt(digitalPinToInterrupt(PIN_LEFT), handleLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_CENTER), handleCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_RIGHT), handleRightPress, FALLING);

    //Serial.println("✅ Tlačítka inicializována");
}