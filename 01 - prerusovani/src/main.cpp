#include <Arduino.h>
const int BTN_BUILTIN_1 = 0;   // Vestavěné tlačítko (BOOT)
const int BTN_BUILTIN_2 = 35;  // Druhé vestavěné tlačítko (na pravé straně)

volatile bool btn1Pressed = false;
volatile bool btn2Pressed = false;

// ISR (Interrupt Service Routine) – co se má stát při přerušení
void IRAM_ATTR handleBtn1() {
  btn1Pressed = true;
}

void IRAM_ATTR handleBtn2() {
  btn2Pressed = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_BUILTIN_1, INPUT_PULLUP);
  pinMode(BTN_BUILTIN_2, INPUT_PULLUP);

  // Nastavení přerušení – aktivuje se při přechodu z HIGH na LOW (tedy při stisku tlačítka)
  attachInterrupt(digitalPinToInterrupt(BTN_BUILTIN_1), handleBtn1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_BUILTIN_2), handleBtn2, FALLING);

  Serial.println("Interrupt test - stiskni tlacitka");
}

void loop() {
  // Simulujeme, že program "něco dělá"
  Serial.println("Program bezi...");
  delay(2000);  // ← i během této prodlevy se tlačítko zachytí přes interrupt

  // Zpracování událostí z ISR
  if (btn1Pressed) {
    btn1Pressed = false;
    Serial.println("Tlacitko 1 bylo stisknuto (interrupt)");
  }

  if (btn2Pressed) {
    btn2Pressed = false;
    Serial.println("Tlacitko 2 bylo stisknuto (interrupt)");
  }
}
