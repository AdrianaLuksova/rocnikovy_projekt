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
const unsigned long debounceDelay = 200;

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

// Názvy akcí pro menu
const char* actionNames[] = {"EAT", "SLEEP", "BATH", "PLAY", "HEAL", "INFO"};
// Zkrácené verze pro zobrazení
const char* actionShort[] = {"E", "S", "B", "P", "H", "I"};

// === Prototypy ===
void drawIdle();
void drawMenu();
void playAction(const char *bmp1, const char *bmp2);
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
  uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  
  bmpFS.seek(18);
  int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  
  bmpFS.seek(28);
  uint16_t bitsPerPixel = bmpFS.read() | (bmpFS.read() << 8);

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

// === Vykreslení menu s ikonkami ===
void drawMenu() {
  // Vyčistit pouze oblast menu (spodních 25 pixelů)
  tft.fillRect(0, 110, 240, 25, TFT_BLACK);
  
  int iconWidth = 40; // Každá ikonka 40px široká
  int startX = 0;
  
  for (int i = 0; i < ACTION_COUNT; i++) {
    int x = startX + (i * iconWidth);
    
    // Zvýraznit vybranou akci
    if (i == selectedAction) {
      tft.fillRect(x, 110, iconWidth, 25, TFT_BLUE);
    } else {
      tft.fillRect(x, 110, iconWidth, 25, TFT_DARKGREY);
    }
    
    // Nakreslit border
    tft.drawRect(x, 110, iconWidth, 25, TFT_WHITE);
    
    // Nakreslit text
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x + 12, 115);
    tft.print(actionShort[i]);
  }
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n🐾 Tamagotchi Start!");
  
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.println("Nacitani...");

  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS mount fail");
    tft.fillScreen(TFT_RED);
    tft.setCursor(10, 60);
    tft.println("ERROR!");
    while (1);
  }
  Serial.println("✅ LittleFS OK");

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  
  // Počkat až se tlačítka stabilizují
  delay(100);
  
  attachInterrupt(digitalPinToInterrupt(BTN_LEFT), onLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_RIGHT), onRight, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_OK), onOK, FALLING);
  
  Serial.println("✅ Tlačítka inicializována");
  Serial.printf("   LEFT=%d, OK=%d, RIGHT=%d\n", 
                digitalRead(BTN_LEFT), 
                digitalRead(BTN_OK), 
                digitalRead(BTN_RIGHT));

  prefs.begin("pet", false);
  loadState();
  prefs.end();

  randomSeed(analogRead(0));

  drawIdle();
  drawMenu();
}

// === Main loop ===
void loop() {
  // Debug - ukázat stav tlačítek každých 2 sekundy
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    lastDebug = millis();
    Serial.printf("🔘 BTN status: L=%d O=%d R=%d | flags: L=%d O=%d R=%d\n", 
                  digitalRead(BTN_LEFT), 
                  digitalRead(BTN_OK), 
                  digitalRead(BTN_RIGHT),
                  leftPressed, okPressed, rightPressed);
  }
  
  // Decay stavů
  if (millis() - lastDecay > decayInterval) {
    lastDecay = millis();
    hunger = max(0, hunger - 1);
    sleepiness = max(0, sleepiness - 1);
    hygiene = max(0, hygiene - 1);
    health = max(0, health - (sick ? 2 : 0));
    saveState();
  }

  checkIllness();

  // Tlačítko VLEVO
  if (leftPressed) {
    leftPressed = false;
    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    Serial.printf("⬅️ Vybrána akce: %s (index %d)\n", actionNames[selectedAction], selectedAction);
    drawMenu();
  }

  // Tlačítko VPRAVO
  if (rightPressed) {
    rightPressed = false;
    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    Serial.printf("➡️ Vybrána akce: %s (index %d)\n", actionNames[selectedAction], selectedAction);
    drawMenu();
  }

  // Tlačítko OK
  if (okPressed) {
    okPressed = false;
    Serial.printf("✅ Spouštím akci: %s\n", actionNames[selectedAction]);

    switch (selectedAction) {
      case 0: // JÍDLO
        playAction("/eat1.bmp", "/eat2.bmp"); 
        hunger = min(100, hunger + 20); 
        break;
        
      case 1: // SPÁNEK
        playAction("/sleep1.bmp", "/sleep2.bmp"); 
        sleepiness = min(100, sleepiness + 20); 
        break;
        
      case 2: // KOUPEL
        playAction("/bath1.bmp", "/bath2.bmp"); 
        hygiene = min(100, hygiene + 20); 
        break;
        
      case 3: // HRA
        playAction("/play1.bmp", "/play2.bmp"); 
        health = min(100, health + 10);
        hunger = max(0, hunger - 5); // Hraní vyčerpává
        break;
        
      case 4: // LÉČBA
        if (sick) {
          playAction("/heal1.bmp", "/heal2.bmp");
          sick = false;
          health = min(100, health + 30);
          Serial.println("💊 Vyléčeno!");
        } else {
          playAction("/heal1.bmp", "/heal2.bmp");
          Serial.println("💊 Není nemocný");
        }
        break;
        
      case 5: // STAV
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setCursor(10, 10);
        tft.println("=== STAV ===");
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(10, 35);
        tft.printf("Hlad:    %d%%\n", hunger);
        tft.printf("Spanek:  %d%%\n", sleepiness);
        tft.printf("Hygiena: %d%%\n", hygiene);
        tft.printf("Zdravi:  %d%%\n", health);
        
        if (sick) {
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.setCursor(10, 95);
          tft.println("NEMOCNY!");
        }
        
        delay(3000);
        break;
    }

    saveState();
    drawIdle();
    drawMenu();
  }

  delay(10);
}

// === Funkce ===
void drawIdle() {
  tft.fillRect(0, 0, 240, 110, TFT_BLACK); // Vyčistit pouze oblast pro obrázek
  
  if (sick) {
    if (!drawBmp("/sick1.bmp", 0, 0)) {
      tft.setCursor(60, 50);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(3);
      tft.println("🤒");
    }
  } else {
    if (!drawBmp("/base1.bmp", 0, 0)) {
      tft.setCursor(60, 50);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setTextSize(3);
      tft.println("🐾");
    }
  }
}

void playAction(const char *bmp1, const char *bmp2) {
  for (int i = 0; i < 3; i++) {
    tft.fillRect(0, 0, 240, 110, TFT_BLACK);
    drawBmp(bmp1, 0, 0);
    delay(300);
    tft.fillRect(0, 0, 240, 110, TFT_BLACK);
    drawBmp(bmp2, 0, 0);
    delay(300);
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
    
    // Větší šance na nemoc pokud jsou špatné hodnoty
    int illChance = 7;
    if (hunger < 30) illChance -= 1;
    if (sleepiness < 30) illChance -= 1;
    if (hygiene < 30) illChance -= 2;
    
    if (random(0, 10) > illChance) {
      sick = true;
      playAction("/sick1.bmp", "/sick2.bmp");
      saveState();
      Serial.println("🤒 Mazlíček onemocněl!");
    }
    nextIllTime = random(60000, 180000);
  }
}
