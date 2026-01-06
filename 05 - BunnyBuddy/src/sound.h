#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

void initSound();
void playClick();
void playSuccess();
void playAlarm();

void playStart();
void playCancel();
void playTone(int freq, int duration); 

#endif
