#include "sound.h"
#include <EasyBuzzer.h> // Použijeme knihovnu

// initSound už nepotřebujeme dělat složitě, 
// protože setPin jsme volali v main.cpp
void initSound() {
    // Prázdné, nebo pro jistotu:
    EasyBuzzer.stopBeep();
}

void playTone(int freq, int duration) {
    // Jednoduché pípnutí (frekvence, délka)
    EasyBuzzer.singleBeep(freq, duration);
}

// Cinknutí při startu
void playStart() {
    // Vysoké pípnutí (2000Hz, 100ms)
    // Knihovna sama zařídí zapnutí i vypnutí
    EasyBuzzer.singleBeep(2000, 100);
}

// Zvuk zrušení akce
void playCancel() {
    // Hluboké pípnutí (200Hz, 300ms)
    EasyBuzzer.singleBeep(200, 300);
}

// Siréna (nemoc / nula)
void playAlarm() {
    // EasyBuzzer umí sekvence!
    // beep(frekvence, délka_zapnuti, délka_vypnuti, počet_opakovani)
    // 3x pípne na 1000Hz
    EasyBuzzer.beep(1000, 200, 100, 3, 200, 1, nullptr);
}