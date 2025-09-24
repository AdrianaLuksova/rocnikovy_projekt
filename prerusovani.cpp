#include <Arduino.h>

const int buttonPin1 = 12; //1
const int buttonPin2 = 13;  //2

// kontorla jestli stisknuti probehlo
bool button1Pressed = false;
bool button2Pressed = false;

void setup() {
  
  pinMode(buttonPin1, INPUT_PULLUP);  
  pinMode(buttonPin2, INPUT_PULLUP);  

  
  Serial.begin(115200);
}

void loop() {
  // Zkontroluju, jestli bylo tlačítko 1 stisknuto
  if (digitalRead(buttonPin1) == LOW) {  // jestli je na pinu 12 nízký signál (tlačítko stisknuté)
    if (!button1Pressed) {  // Pokud tlačítko ještě nebylo zaznamenáno
      Serial.println("Tlačítko 1 bylo stisknuto!");
      button1Pressed = true;  // Zapamatujeme si ze jo
    }
  } else {
    button1Pressed = false;  // Pokud ne - resetuj stav
  }

  // Zkontroluju jestli tlačítko 2 stisknuto
  if (digitalRead(buttonPin2) == LOW) {  // Pokud je na pinu 13 nízký signál (tlačítko stisknuté)
    if (!button2Pressed) {  // Pokud tlačítko ještě nebylo 
      Serial.println("Tlačítko 2 bylo stisknuto!");
      button2Pressed = true;  // Zapamatujeme si že stisknuto
    }
  } else {
    button2Pressed = false;  // Pokud tlačítko není stisknuté, resetuj stav
  }
}
