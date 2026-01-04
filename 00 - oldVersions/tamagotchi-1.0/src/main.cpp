#include <Arduino.h>
#include <TFT_eSPI.h>
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
void drawStatus();
void saveState();
void loadState();
void checkIllness();
void drawBattery();

// === Přerušení ===
void IRAM_ATTR onLeft() { leftPressed = true; }
void IRAM_ATTR onRight() { rightPressed = true; }
void IRAM_ATTR onOK() { okPressed = true; }

// === BMP načítání ===
void drawBmp(const char *filename, int16_t x, int16_t y) {
  File bmpFS = LittleFS.open(filename, "r");
  if (!bmpFS) return;
  uint16_t w, h;
  uint32_t seekOffset;
  bmpFS.seek(10);
  seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
  bmpFS.seek(18);
  w = bmpFS.read() | (bmpFS.read() << 8);
  h = bmpFS.read() | (bmpFS.read() << 8);
  bmpFS.seek(seekOffset);
  uint8_t r, g, b;
  for (int row = h - 1; row >= 0; row--) {
    for (int col = 0; col < w; col++) {
      b = bmpFS.read();
      g = bmpFS.read();
      r = bmpFS.read();
      tft.drawPixel(x + col, y + row, tft.color565(r, g, b));
    }
  }
  bmpFS.close();
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

void drawStatus() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextSize(1);
  tft.println("📊 STAV MAZLÍČKA");

  int barX = 40, barY = 30, barW = 150, barH = 10, gap = 20;

  tft.setCursor(10, barY);
  tft.print("🍖");
  drawBar(barX, barY, barW, barH, hunger);

  barY += gap;
  tft.setCursor(10, barY);
  tft.print("😴");
  drawBar(barX, barY, barW, barH, sleepiness);

  barY += gap;
  tft.setCursor(10, barY);
  tft.print("🧼");
  drawBar(barX, barY, barW, barH, hygiene);

  barY += gap;
  tft.setCursor(10, barY);
  tft.print("❤️");
  drawBar(barX, barY, barW, barH, health);

  if (sick) {
    tft.setCursor(10, barY + gap + 10);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("🤒 Mazlíček je nemocný!");
  }

  drawBattery();
}


// === Setup ===
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS mount fail");
    while (1);
  }

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
  drawStatus();
}

// === Main loop ===
void loop() {
  if (millis() - lastDecay > decayInterval) {
    lastDecay = millis();
    hunger = max(0, hunger - 1);
    sleepiness = max(0, sleepiness - 1);
    hygiene = max(0, hygiene - 1);
    health = max(0, health - (sick ? 2 : 0));
    saveState();
    drawStatus();
  }

  checkIllness();

  if (leftPressed) {
    leftPressed = false;
    selectedAction = (selectedAction - 1 + ACTION_COUNT) % ACTION_COUNT;
    drawStatus();
  }

  if (rightPressed) {
    rightPressed = false;
    selectedAction = (selectedAction + 1) % ACTION_COUNT;
    drawStatus();
  }

  if (okPressed) {
    okPressed = false;

    switch (selectedAction) {
      case 0: playAction("/eat1.bmp", "/eat2.bmp"); hunger = min(100, hunger + 10); break;
      case 1: playAction("/sleep1.bmp", "/sleep2.bmp"); sleepiness = min(100, sleepiness + 10); break;
      case 2: playAction("/bath1.bmp", "/bath2.bmp"); hygiene = min(100, hygiene + 10); break;
      case 3: playAction("/play1.bmp", "/play2.bmp"); health = min(100, health + 5); break;
      case 4:
        if (sick) {
          playAction("/heal1.bmp", "/heal2.bmp");
          sick = false;
          health = min(100, health + 20);
        } else playAction("/heal1.bmp", "/heal2.bmp");
        break;
      case 5: drawStatus(); break; // status
    }

    saveState();
    drawIdle();
    drawStatus();
  }
}

// === Funkce ===
void drawIdle() {
  tft.fillScreen(TFT_BLACK);
  drawBmp("/base1.bmp", 0, 0);
}

void playAction(const char *bmp1, const char *bmp2) {
  for (int i = 0; i < 3; i++) {
    drawBmp(bmp1, 0, 0);
    delay(250);
    drawBmp(bmp2, 0, 0);
    delay(250);
  }
}

void drawStatus() {
  tft.fillRect(0, 120, 240, 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(5, 125);
  tft.printf("🍖%d 😴%d 🧼%d ❤️%d", hunger, sleepiness, hygiene, health);
  if (sick) tft.setTextColor(TFT_RED, TFT_BLACK), tft.print(" 🤒");
  drawBattery();
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

void drawBattery() {
  int raw = analogRead(34); // pokud měříš napětí přes dělič
  float voltage = raw / 4095.0 * 2 * 3.3;
  int pct = map(voltage * 100, 360, 420, 0, 100);
  pct = constrain(pct, 0, 100);
  tft.setCursor(180, 125);
  tft.printf("🔋%d%%", pct);
}
