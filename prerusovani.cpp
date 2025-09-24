#include <Arduino.h>

// Piny pro tlačítka
const int buttonPin1 = 12;  // Tlačítko 1 na pinu 12
const int buttonPin2 = 13;  // Tlačítko 2 na pinu 13

// Proměnné pro kontrolu, jestli bylo tlačítko stisknuto
bool button1Pressed = false;
bool button2Pressed = false;

void setup() {
  // Nastavíme piny pro tlačítka jako vstupy
  pinMode(buttonPin1, INPUT_PULLUP);  // Tlačítko 1 s pull-up rezistorem
  pinMode(buttonPin2, INPUT_PULLUP);  // Tlačítko 2 s pull-up rezistorem

  // Spustíme sériovou komunikaci pro výstup
  Serial.begin(115200);
}

void loop() {
  // Zkontrolujeme, jestli bylo tlačítko 1 stisknuto
  if (digitalRead(buttonPin1) == LOW) {  // Pokud je na pinu 12 nízký signál (tlačítko stisknuté)
    if (!button1Pressed) {  // Pokud tlačítko ještě nebylo zaznamenáno
      Serial.println("Tlačítko 1 bylo stisknuto!");
      button1Pressed = true;  // Zapamatujeme si, že tlačítko bylo stisknuto
    }
  } else {
    button1Pressed = false;  // Pokud tlačítko není stisknuté, resetuj stav
  }

  // Zkontrolujeme, jestli bylo tlačítko 2 stisknuto
  if (digitalRead(buttonPin2) == LOW) {  // Pokud je na pinu 13 nízký signál (tlačítko stisknuté)
    if (!button2Pressed) {  // Pokud tlačítko ještě nebylo zaznamenáno
      Serial.println("Tlačítko 2 bylo stisknuto!");
      button2Pressed = true;  // Zapamatujeme si, že tlačítko bylo stisknuto
    }
  } else {
    button2Pressed = false;  // Pokud tlačítko není stisknuté, resetuj stav
  }
}
