#include "sound.h"
#include <Arduino.h>

// Zkontroluj svůj PIN (25 nebo jiný)
const int PIN_BUZZER = 26; 

void initSound() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW); // Ticho na startu
}

void updateSound() {
    // Tady nic není, "hloupá" metoda nepotřebuje update
}

// === NAŠE VLASTNÍ HLOUPÁ FUNKCE (BRUTE FORCE) ===
// Ručně kmitáme pinem. Žádné timery, žádné konflikty.
void myTone(int freq, int duration) {
    // Pojistka proti dělení nulou
    if (freq == 0) return; 

    // Vypočítáme, jak rychle musíme blikat (perioda v mikrosekundách)
    // 1 sekunda = 1 000 000 mikrosekund
    long delayValue = 1000000 / freq / 2; 

    // Vypočítáme, kolikrát to musíme zopakovat, aby to trvalo 'duration' milisekund
    long numCycles = (long)freq * duration / 1000;

    for (long i = 0; i < numCycles; i++) {
        digitalWrite(PIN_BUZZER, HIGH); // Zapni proud
        delayMicroseconds(delayValue);  // Počkej malou chviličku
        digitalWrite(PIN_BUZZER, LOW);  // Vypni proud
        delayMicroseconds(delayValue);  // Počkej
    }
    
    // NA KONCI VŽDY NATVRDO VYPNEME
    // Tím zmizí to "hučení"
    digitalWrite(PIN_BUZZER, LOW); 
}

// 1. POTVRZENÍ (Ostřejší tón)
void playStart() {
    myTone(2500, 80); 
}

// 2. PŘERUŠENÍ / CHYBA (Hlubší tón)
void playCancel() {
    myTone(800, 200);
}

// 3. ÚSPĚCH (Melodie)
void playSuccess() {
    myTone(2000, 100);
    delay(50); // Krátké ticho
    myTone(3000, 150);
}

// 4. ALARM (Výstraha)
void playAlarm() {
    for(int i=0; i<3; i++) {
        myTone(4000, 150);
        delay(100);
    }
}

// Obecná funkce pro volání z pet.h
void playTone(int freq, int duration) {
    myTone(freq, duration);
}