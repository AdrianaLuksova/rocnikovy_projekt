#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

void initSound();
void playTone(int freq, int duration);
void playStart();  // Cinknutí na začátku
void playCancel(); // Hluboké pípnutí při zrušení
void playAlarm();  // Siréna

#endif