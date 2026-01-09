#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

// Inicializace zvuku
void initSound();

// Hlavní smyčka pro zvuk (musí být v loopu)
void updateSound();

// Konkrétní zvuky
void playStart();    // Potvrzení akce (Cink!)
void playCancel();   // Přerušení/Chyba (Bzzzt)
void playSuccess();  // Úspěch (Tadá!)
void playAlarm();    // Kritický stav (Píp píp píp!)

// Pomocná funkce pro hraní tónu
void playTone(int freq, int duration);

#endif