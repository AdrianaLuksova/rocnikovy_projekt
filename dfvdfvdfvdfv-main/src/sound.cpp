#include "sound.h"
#include <Arduino.h>

#define BUZZER_PIN 27

// Funkce pro "vynucené ticho"
void stopSound() {
    digitalWrite(BUZZER_PIN, LOW); // Okamžitě vypnout pin
}

void initSound() {
    pinMode(BUZZER_PIN, OUTPUT);
    stopSound(); // Zajistit ticho na startu

    // Krátký test při startu - pokud tohle nezazní, je chyba v zapojení
    playTone(2000, 100); 
}

// Manuální generování tónu (Funguje na všech typech ESP32)
void playTone(int freq, int duration) {
    if (freq <= 0 || duration <= 0) {
        stopSound();
        return;
    }

    // Výpočet periody vlny
    long delayValue = 1000000 / freq / 2; 
    long numCycles = freq * duration / 1000;

    for (long i = 0; i < numCycles; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(delayValue);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(delayValue);
    }
    
    // DŮLEŽITÉ: Po skončení smyčky okamžitě vypnout!
    // Tohle je ten řádek, který ti chyběl a způsoboval konstantní bzučení
    stopSound(); 
}

// === ZVUKY PRO HRU (Upraveno pro jasné "cinknutí") ===

void playClick() {
    // Velmi krátké cvaknutí
    playTone(3500, 15); 
}

void playStart() {
    // "Tud-lú" - start akce
    playTone(1500, 80);
    delay(30);
    playTone(2500, 100);
}

void playCancel() {
    // "Brum-brum" - zrušení
    playTone(800, 100);
    delay(50);
    playTone(500, 100);
}

void playSuccess() {
    // "Cink-Cink!" - úspěch
    playTone(2000, 80);
    delay(50);
    playTone(4000, 150);
}

void playAlarm() {
    // Siréna - jen 3x a dost
    for (int i = 0; i < 3; i++) {
        playTone(2000, 150);
        delay(100);
        playTone(1500, 150);
        delay(100);
    }
    stopSound(); // Pro jistotu
}