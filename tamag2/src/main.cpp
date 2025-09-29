#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

#define TFT_BL 4  // pin podsvícení

// Připoj tvůj .c soubor s polem "pozadi"
#include "pozadi.c"  // musí obsahovat: const uint16_t pozadi[240*135]

void setup() {
  Serial.begin(115200);

  // Zapnutí podsvícení
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Inicializace TFT
  tft.init();
  tft.setRotation(3);

  // Pokud je obrázek v RGB565 v opačném pořadí
  tft.setSwapBytes(false);

  // Vykreslení pozadí z externího .c souboru
  tft.pushImage(0, 0, 240, 135, pozadi);

  // Přidání jednoduché Tamagochi postavičky
}

void loop() {
  // Sem můžeš přidat animace nebo interakce
}
