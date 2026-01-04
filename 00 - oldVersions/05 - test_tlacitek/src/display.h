#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>

extern TFT_eSPI tft;
extern volatile int selectedAction;
extern const int ACTION_COUNT;
extern const char* actionShort[];
extern bool sick;

// === Inicializace displeje ===
void initDisplay() {
  // Podsvícení
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  // Display init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  Serial.println("✅ Display inicializován");
}

// === BMP načítání ===
bool drawBmp(const char *filename, int16_t x, int16_t y) {
  File bmpFS = LittleFS.open(filename, "r");
  if (!bmpFS) {
    Serial.printf("❌ Nelze otevřít: %s\n", filename);
    return false;
  }

  if (bmpFS.read() != 'B' || bmpFS.read() != 'M') {
    Serial.printf("❌ Není BMP: %s\n", filename);
    bmpFS.close();
    return false;
  }

  bmpFS.seek(10);
  uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | 
                        (bmpFS.read() << 16) | (bmpFS.read() << 24);
  
  bmpFS.seek(18);
  int32_t w = bmpFS.read() | (bmpFS.read() << 8) | 
              (bmpFS.read() << 16) | (bmpFS.read() << 24);
  int32_t h = bmpFS.read() | (bmpFS.read() << 8) | 
              (bmpFS.read() << 16) | (bmpFS.read() << 24);

  uint32_t rowSize = ((w * 3 + 3) / 4) * 4;
  bmpFS.seek(seekOffset);
  
  uint8_t r, g, b;
  for (int row = h - 1; row >= 0; row--) {
    for (int col = 0; col < w; col++) {
      b = bmpFS.read();
      g = bmpFS.read();
      r = bmpFS.read();
      tft.drawPixel(x + col, y + row, tft.color565(r, g, b));
    }
    for (uint32_t pad = w * 3; pad < rowSize; pad++) {
      bmpFS.read();
    }
  }
  
  bmpFS.close();
  return true;
}

// === Vykreslení menu ===
void drawMenu() {
  tft.fillRect(0, 110, 240, 25, TFT_BLACK);
  
  int iconWidth = 40;
  
  for (int i = 0; i < ACTION_COUNT; i++) {
    int x = i * iconWidth;
    
    if (i == selectedAction) {
      tft.fillRect(x, 110, iconWidth, 25, TFT_BLUE);
    } else {
      tft.fillRect(x, 110, iconWidth, 25, TFT_DARKGREY);
    }
    
    tft.drawRect(x, 110, iconWidth, 25, TFT_WHITE);
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x + 12, 115);
    tft.print(actionShort[i]);
  }
}

// === Vykreslení idle stavu ===
void drawIdle() {
  tft.fillRect(0, 0, 240, 110, TFT_BLACK);
  
  if (sick) {
    if (!drawBmp("/sick1.bmp", 0, 0)) {
      tft.setCursor(90, 45);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(4);
      tft.println(":(");
    }
  } else {
    if (!drawBmp("/base1.bmp", 0, 0)) {
      tft.setCursor(90, 45);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(4);
      tft.println(":)");
    }
  }
}

// === Zobrazení loading screen ===
void showLoading() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(40, 60);
  tft.println("Loading...");
}

// === Zobrazení chyby ===
void showError(const char* message) {
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 60);
  tft.println(message);
}

#endif