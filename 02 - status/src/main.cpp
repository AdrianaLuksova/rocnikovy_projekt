#include <Arduino.h>

#include <Preferences.h>

const int BTN_JIDLO = 0;    // BOOT
const int BTN_SPANEK = 35;  // druhé vestavěné

volatile bool jidloStisk = false;
volatile bool spanekStisk = false;

int jidlo = 100;
int spanek = 100;

const int MAX_HODNOTA = 100;
const int PRIDAT_HODNOTA = 10;
const int UBIRAT_HODNOTA = 1;

unsigned long posledniUbytek = 0;
const unsigned long intervalUbytku = 10000;

Preferences prefs;

// 👉 DOPLNĚNO – deklarace funkcí
void vypisStav();
void ulozStav();

void IRAM_ATTR stiskJidlo() {
  jidloStisk = true;
}

void IRAM_ATTR stiskSpanek() {
  spanekStisk = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_JIDLO, INPUT_PULLUP);
  pinMode(BTN_SPANEK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_JIDLO), stiskJidlo, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_SPANEK), stiskSpanek, FALLING);

  prefs.begin("zvire", false);
  jidlo = prefs.getInt("jidlo", 100);
  spanek = prefs.getInt("spanek", 100);
  prefs.end();

  Serial.println("🦊 Zaciname mazlicka!");
  vypisStav();
}

void loop() {
  if (millis() - posledniUbytek >= intervalUbytku) {
    posledniUbytek = millis();
    jidlo = max(0, jidlo - UBIRAT_HODNOTA);
    spanek = max(0, spanek - UBIRAT_HODNOTA);
    ulozStav();
    Serial.println("📉 Potreby ubyly o 1");
    vypisStav();
  }

  if (jidloStisk) {
    jidloStisk = false;
    jidlo = min(MAX_HODNOTA, jidlo + PRIDAT_HODNOTA);
    ulozStav();
    Serial.println("🍖 Jidlo pridano!");
    vypisStav();
    delay(200);
  }

  if (spanekStisk) {
    spanekStisk = false;
    spanek = min(MAX_HODNOTA, spanek + PRIDAT_HODNOTA);
    ulozStav();
    Serial.println("🛌 Spanek pridany!");
    vypisStav();
    delay(200);
  }
}

// 👉 DEFINICE FUNKCÍ

void vypisStav() {
  Serial.printf("📊 Stav mazlicka: JIDLO: %d / SPANEK: %d\n", jidlo, spanek);
}

void ulozStav() {
  prefs.begin("zvire", false);
  prefs.putInt("jidlo", jidlo);
  prefs.putInt("spanek", spanek);
  prefs.end();
}
