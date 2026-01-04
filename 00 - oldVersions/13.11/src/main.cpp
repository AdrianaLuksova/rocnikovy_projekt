#include <Arduino.h>
#include <TFT_eSPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>

// === Globální objekty ===
TFT_eSPI tft = TFT_eSPI();
Preferences prefs;

// === Globální proměnné ===
volatile bool actionInProgress = false;
volatile int selectedAction = 0;
volatile bool needsRedraw = false;
const int ACTION_COUNT = 6;

int hunger = 100;
int sleepiness = 100;
int hygiene = 100;
int health = 100;
bool sick = false;

unsigned long lastDecay = 0;
const unsigned long decayInterval = 10000;

unsigned long lastIllCheck = 0;
unsigned long nextIllTime = 60000;

const char* actionNames[] = {"EAT", "SLEEP", "BATH", "PLAY", "HEAL", "INFO"};
const char* actionShort[] = {"E", "S", "B", "P", "H", "I"};

// === Include hlavičkových souborů ===
#include "buttons.h"
#include "display.h"
#include "pet.h"

// === Setup ===
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== TAMAGOTCHI START ===");
  
  // Inicializace displeje
  initDisplay();
  showLoading();
  
  // Inicializace LittleFS
  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS FAIL");
    showError("FS ERROR!");
    while (1);
  }
  Serial.println("✅ LittleFS OK");
  
  // Debug: Výpis souborů
  Serial.println("\n📁 Soubory v LittleFS:");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.printf("  - %s (%d bytes)\n", file.name(), file.size());
    file = root.openNextFile();
  }
  Serial.println();
  
  // Inicializace tlačítek
  initButtons();
  
  // Načtení uloženého stavu
  loadState();
  
  // Random seed
  randomSeed(analogRead(0));
  
  // První zobrazení
  Serial.println("🎨 První vykreslení...");
  drawBackground();  // Pozadí
  drawIdle();        // Králíček
  drawMenu();        // Menu
  
  Serial.println("✅ Setup dokončen\n");
}

// === Main Loop ===
void loop() {
  static unsigned long lastLoop = 0;
  
  // Decay stavů
  updateDecay();
  
  // Kontrola nemoci
  checkIllness();
  
  // Překreslit menu pokud potřeba
  if (needsRedraw) {
    Serial.println("🔄 Překreslování menu...");
    needsRedraw = false;
    drawMenu();
  }
  
  // Provést akci
  if (actionInProgress) {
    Serial.printf("\n▶️ Spouštím akci: %s\n", actionNames[selectedAction]);
    executeAction(selectedAction);
    actionInProgress = false;
    needsRedraw = true;
    Serial.println("⏹️ Akce dokončena\n");
  }
  
  delay(10);
}