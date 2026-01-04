#include <Arduino.h>

// Zkus různé kombinace pinů
const int BTN_1 = 26;
const int BTN_2 = 27;
const int BTN_3 = 2;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n🔍 TEST TLAČÍTEK");
  Serial.println("==================\n");
  
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  
  Serial.printf("📍 Testované piny:\n");
  Serial.printf("   BTN_1 = GPIO %d\n", BTN_1);
  Serial.printf("   BTN_2 = GPIO %d\n", BTN_2);
  Serial.printf("   BTN_3 = GPIO %d\n", BTN_3);
  Serial.println("\n⚡ Klidový stav (1 = nestisknuto, 0 = stisknuto):");
  Serial.printf("   BTN_1=%d, BTN_2=%d, BTN_3=%d\n\n", 
                digitalRead(BTN_1),
                digitalRead(BTN_2),
                digitalRead(BTN_3));
  
  Serial.println("🎮 Stiskni tlačítka...\n");
}

void loop() {
  int val1 = digitalRead(BTN_1);
  int val2 = digitalRead(BTN_2);
  int val3 = digitalRead(BTN_3);
  
  // Zobrazit když se něco změní
  static int last1 = 1, last2 = 1, last3 = 1;
  
  if (val1 != last1) {
    Serial.printf("🔴 BTN_1 (GPIO %d): %s\n", BTN_1, val1 == 0 ? "STISKNUTO ✅" : "uvolněno");
    last1 = val1;
  }
  
  if (val2 != last2) {
    Serial.printf("🟢 BTN_2 (GPIO %d): %s\n", BTN_2, val2 == 0 ? "STISKNUTO ✅" : "uvolněno");
    last2 = val2;
  }
  
  if (val3 != last3) {
    Serial.printf("🔵 BTN_3 (GPIO %d): %s\n", BTN_3, val3 == 0 ? "STISKNUTO ✅" : "uvolněno");
    last3 = val3;
  }
  
  delay(50);
}