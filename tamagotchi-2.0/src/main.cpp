#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>

// === TFT ===
TFT_eSPI tft = TFT_eSPI();

// === Piny tlačítek ===
const int BTN_LEFT = 32;
const int BTN_OK = 33;
const int BTN_RIGHT = 25;

// === Přerušení ===
volatile bool leftPressed = false;
volatile bool rightPressed = false;
volatile bool okPressed = false;
volatile unsigned long lastLeftPress = 0;
volatile unsigned long lastRightPress = 0;
volatile unsigned long lastOkPress = 0;
const unsigned long debounceDelay = 200; // ms

// === Stav ===
Preferences prefs;
int hunger = 100;
int sleepiness = 100;
int hygiene = 100;
int health = 100;
bool sick = false;

unsigned long lastDecay = 0;
const unsigned long decayInterval = 10000;

unsigned long lastIllCheck = 0;
unsigned long nextIllTime = 60000;

int selectedAction = 0;
const int ACTION_COUNT = 6;

// === Prototypy ===
void drawIdle();
void playAction(const char *bmp1, const char *bmp2);
void drawStatusBar();
void saveState();
void loadState();
void checkIllness();

// === Přerušení ===
void IRAM_ATTR onLeft() { 
  unsigned long now = millis();
  if (now - lastLeftPress > debounceDelay) {
    leftPressed = true;
    lastLeftPress = now;
  }
}

void IRAM_ATTR onRight() { 
  unsigned long now = millis();
  if (now - lastRightPress > debounceDelay) {
    rightPressed = true;
    lastRightPress = now;
  }
}

void IRAM_ATTR onOK() { 
  unsigned long now = millis();
  if (now - lastOkPress > debounceDelay) {
    okPressed = true;
    lastOkPress = now;
  }
}

// === OPRAVENÉ BMP načítání ===
bool drawBmp(const char *filename, int16_t x, int16_t y) {
  File bmpFS = LittleFS.open(filename, "r");
  if (!bmpFS) {
    Serial.printf("❌ Nelze otevřít: %s\n", filename);
    return false;
  }

  // Kontrola BMP hlavičky
  if (bmpFS.read() != 'B' || bmpFS.read() != 'M') {
    Serial.printf("❌ Není BMP: %s\n", filename);
    bmpFS.close();
    return false;
  }

  // Přečtení offset dat (kde začínají pixely)
  bmpFS.seek(10);
  uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  
  // Přečtení rozměrů
  bmpFS.seek(18);
  int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  
  // Přečtení počtu bitů na pixel
  bmpFS.seek(28);
  uint16_t bitsPerPixel = bmpFS.read() | (bmpFS.read() << 8);
  
  Serial.printf("📷 BMP: %s (%dx%d, %d bpp)\n", filename, w, h, bitsPerPixel);
  Serial.printf("   Display: %dx%d\n", tft.width(), tft.height());

 // if (bitsPerPixel != 24) {
  //  Serial.printf("❌ Podporováno pouze 24-bit BMP!\n");
   // bmpFS.close();
    //return false;  
    //  }

  // Výpočet paddingu řádku (musí být násobek 4)
  uint32_t rowSize = ((w * 3 + 3) / 4) * 4;
  
  // Skok na data pixelů
  bmpFS.seek(seekOffset);
  
  // Načtení pixelů (BMP je uložené od spodu nahoru)
  uint8_t r, g, b;
  for (int row = h - 1; row >= 0; row--) {
    for (int col = 0; col < w; col++) {
      b = bmpFS.read();
      g = bmpFS.read();
      r = bmpFS.read();
      tft.drawPixel(x + col, y + row, tft.color565(r, g, b));
    }
    // Přeskočení paddingu na konci řádku
    for (uint32_t pad = w * 3; pad < rowSize; pad++) {
      bmpFS.read();
    }
  }
  
  bmpFS.close();
 // Serial.printf("✅ BMP načten: %s\n", filename);
  return true;
}

void drawBar(int x, int y, int width, int height, int value) {
  uint16_t color;
  if (value > 66) color = TFT_GREEN;
  else if (value > 33) color = TFT_ORANGE;
  else color = TFT_RED;

  int filled = map(value, 0, 100, 0, width);
  tft.drawRect(x, y, width, height, TFT_WHITE);
  tft.fillRect(x+1, y+1, filled-2, height-2, color);
}

void drawStatusBar() {
  tft.fillRect(0, 120, 240, 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(5, 125);
  tft.printf("🍖%d 😴%d 🧼%d ❤️%d", hunger, sleepiness, hygiene, health);
  if (sick) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print(" 🤒");
  }
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n🐾 Tamagotchi Start!");
  
  // Zapnout podsvícení
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  tft.init();
  tft.setRotation(1); // 1 = landscape (240x135)

  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.println("Nacitani...");

  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS mount fail");
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 60);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.println("LittleFS ERROR!");
    while (1);
  }
  Serial.println("✅ LittleFS OK");

  // Test existence souborů
  // const char* testFiles[] = {"/base1.bmp", "/eat1.bmp", "/eat2.bmp", 
  //                            "/sleep1.bmp", "/sleep2.bmp", "/bath1.bmp",
  //                            "/bath2.bmp", "/play1.bmp", "/play2.bmp",
  //                            "/heal1.bmp", "/heal2.bmp", "/sick1.bmp", "/sick2.bmp"};

  // for (const char* file : testFiles) {
  //  if (LittleFS.exists(file)) {
  //    Serial.printf("✅ Nalezen: %s\n", file);
  //  } else {
  //    Serial.printf("❌ Chybí: %s\n", file);
  //  }
 // }

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  attachInterrupt(BTN_LEFT, onLeft, FALLING);
  attachInterrupt(BTN_RIGHT, onRight, FALLING);
  attachInterrupt(BTN_OK, onOK, FALLING);

  prefs.begin("pet", false);
  loadState();
  prefs.end();

  randomSeed(analogRead(0));

  drawIdle();
  drawStatusBar();
}

// === Main loop ===
void loop() {
  // Kontrola decay stavů
  if (millis() - lastDecay > decayInterval) {
    lastDecay = millis();
    hunger = max(0, hunger - 1);
    sleepiness = max(0, sleepiness - 1);
    hygiene = max(0, hygiene - 1);
    health = max(0, health - (sick ? 2 : 0));
    saveState();
    drawStatusBar();
  }

  // Kontrola nemoci
  checkIllness();

  // Zpracování přerušení tlačítek
  if (leftPressed) {
    leftPressed = false;
    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    Serial.printf("⬅️ Akce: %d\n", selectedAction);
    
    // Vizuální feedback
    tft.fillRect(0, 110, 240, 10, TFT_BLUE);
    delay(50);
    drawStatusBar();
  }

  if (rightPressed) {
    rightPressed = false;
    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    Serial.printf("➡️ Akce: %d\n", selectedAction);
    
    // Vizuální feedback
    tft.fillRect(0, 110, 240, 10, TFT_GREEN);
    delay(50);
    drawStatusBar();
  }

  if (okPressed) {
    okPressed = false;
    Serial.printf("✅ Spustit akci: %d\n", selectedAction);
    
    // Vizuální feedback
    tft.fillRect(0, 110, 240, 10, TFT_YELLOW);
    delay(100);

    // Provedení akce
    switch (selectedAction) {
      case 0: 
        playAction("/eat1.bmp", "/eat2.bmp"); 
        hunger = min(100, hunger + 10); 
        break;
      case 1: 
        playAction("/sleep1.bmp", "/sleep2.bmp"); 
        sleepiness = min(100, sleepiness + 10); 
        break;
      case 2: 
        playAction("/bath1.bmp", "/bath2.bmp"); 
        hygiene = min(100, hygiene + 10); 
        break;
      case 3: 
        playAction("/play1.bmp", "/play2.bmp"); 
        health = min(100, health + 5); 
        break;
      case 4:
        if (sick) {
          playAction("/heal1.bmp", "/heal2.bmp");
          sick = false;
          health = min(100, health + 20);
        } else {
          playAction("/heal1.bmp", "/heal2.bmp");
        }
        break;
      case 5: 
        // Zobrazit plný status
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 10);
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.println("STAV:");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.printf("\nHlad: %d%%\n", hunger);
        tft.printf("Spanek: %d%%\n", sleepiness);
        tft.printf("Hygiena: %d%%\n", hygiene);
        tft.printf("Zdravi: %d%%\n", health);
        if (sick) {
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.println("\nNEMOCNY!");
        }
        delay(3000);
        break;
    }

    saveState();
    drawIdle();
    drawStatusBar();
  }

  // Malá pauza aby ESP neběžel na 100%
  delay(10);
}

// === Funkce ===
void drawIdle() {
  tft.fillScreen(TFT_BLACK);
  if (!drawBmp("/base1.bmp", 0, 0)) {
    // Fallback pokud soubor neexistuje
    tft.setCursor(40, 60);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println("🐾 PET");
  }
}

void playAction(const char *bmp1, const char *bmp2) {
  for (int i = 0; i < 3; i++) {
    tft.fillScreen(TFT_BLACK);
    drawBmp(bmp1, 0, 0);
    delay(250);
    tft.fillScreen(TFT_BLACK);
    drawBmp(bmp2, 0, 0);
    delay(250);
  }
}

void saveState() {
  prefs.begin("pet", false);
  prefs.putInt("hunger", hunger);
  prefs.putInt("sleepiness", sleepiness);
  prefs.putInt("hygiene", hygiene);
  prefs.putInt("health", health);
  prefs.putBool("sick", sick);
  prefs.end();
}

void loadState() {
  hunger = prefs.getInt("hunger", 100);
  sleepiness = prefs.getInt("sleepiness", 100);
  hygiene = prefs.getInt("hygiene", 100);
  health = prefs.getInt("health", 100);
  sick = prefs.getBool("sick", false);
}

void checkIllness() {
  if (!sick && millis() - lastIllCheck > nextIllTime) {
    lastIllCheck = millis();
    if (random(0, 10) > 7) {
      sick = true;
      playAction("/sick1.bmp", "/sick2.bmp");
      saveState();
    }
    nextIllTime = random(60000, 120000);
  }
}

