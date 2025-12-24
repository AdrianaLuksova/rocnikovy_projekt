#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// === KONSTANTY ===
const int PIN_LEFT = 32;
const int PIN_CENTER = 33;
const int PIN_RIGHT = 25;
const int ACTION_COUNT = 6;
const int DEBOUNCE_DELAY_MS = 200; 

// === Globální proměnné ===
extern volatile bool actionInProgress;
extern volatile int selectedAction;
extern volatile bool needsRedraw;

// === Funkce ===
void initButtons();

#endif