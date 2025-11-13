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

// === Struktura pro oblast obrázku ===
struct ImageArea {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    uint16_t* buffer;  // uložené pixely pozadí
};

// === Inicializace displeje ===
void initDisplay() {
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    Serial.println("✅ Display inicializován");
}

// === Transparentní BMP kreslení (magenta = průhledná) ===
bool drawBmpTransparent(const char *filename, int16_t x, int16_t y) {
    File bmpFS = LittleFS.open(filename, "r");
    if(!bmpFS) {
        Serial.printf("❌ Nelze otevřít: %s\n", filename);
        return false;
    }

    if(bmpFS.read() != 'B' || bmpFS.read() != 'M') {
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

    uint32_t rowSize = ((w*3+3)/4)*4;
    bmpFS.seek(seekOffset);

    uint8_t r,g,b;
    for(int row=h-1; row>=0; row--) {
        for(int col=0; col<w; col++) {
            b = bmpFS.read();
            g = bmpFS.read();
            r = bmpFS.read();
            // Magenta (255,0,255) = průhledná
            if(!(r==255 && g==0 && b==255)) {
                tft.drawPixel(x+col, y+row, tft.color565(r,g,b));
            }
        }
        for(uint32_t pad=w*3; pad<rowSize; pad++) bmpFS.read();
    }

    bmpFS.close();
    Serial.printf("✅ Nakreslen transparent: %s (%dx%d)\n", filename, w, h);
    return true;
}

// === BMP bez průhlednosti (pro pozadí a ikony) ===
bool drawBmp(const char *filename, int16_t x, int16_t y) {
    File bmpFS = LittleFS.open(filename, "r");
    if(!bmpFS) {
        Serial.printf("❌ Nelze otevřít: %s\n", filename);
        return false;
    }

    if(bmpFS.read() != 'B' || bmpFS.read() != 'M') {
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

    uint32_t rowSize = ((w*3+3)/4)*4;
    bmpFS.seek(seekOffset);

    uint8_t r,g,b;
    for(int row=h-1; row>=0; row--) {
        for(int col=0; col<w; col++) {
            b = bmpFS.read();
            g = bmpFS.read();
            r = bmpFS.read();
            tft.drawPixel(x+col, y+row, tft.color565(r,g,b));
        }
        for(uint32_t pad=w*3; pad<rowSize; pad++) bmpFS.read();
    }

    bmpFS.close();
    Serial.printf("✅ Nakreslen: %s (%dx%d)\n", filename, w, h);
    return true;
}

// === Uloží oblast obrazovky do bufferu ===
void saveArea(ImageArea &area) {
    if(area.buffer) {
        delete[] area.buffer;
    }
    area.buffer = new uint16_t[area.w * area.h];
    
    for(int y=0; y<area.h; y++) {
        for(int x=0; x<area.w; x++) {
            area.buffer[y*area.w+x] = tft.readPixel(area.x + x, area.y + y);
        }
    }
    Serial.printf("💾 Uložena oblast: %dx%d na pozici (%d,%d)\n", area.w, area.h, area.x, area.y);
}

// === Obnoví oblast obrazovky z bufferu ===
void restoreArea(ImageArea &area) {
    if(!area.buffer) {
        Serial.println("⚠️ Buffer je prázdný!");
        return;
    }
    
    for(int y=0; y<area.h; y++) {
        for(int x=0; x<area.w; x++) {
            tft.drawPixel(area.x + x, area.y + y, area.buffer[y*area.w + x]);
        }
    }
    Serial.printf("♻️ Obnovena oblast: %dx%d\n", area.w, area.h);
}

// === Načte pozadí ===
void drawBackground() {
    drawBmp("/pozadi.bmp", 0, 0);
}

// === Idle stav (králíček) ===
void drawIdle() {
    const char* bmpFile = sick ? "/sick1.bmp" : "/base1.bmp";
    drawBmpTransparent(bmpFile, 0, 0);
}

// === Menu s ikonami ===
void drawMenu() {
    int iconWidth = 40;
    int iconHeight = 28;
    int menuY = 110;  // Menu je na spodku (displej je 135px vysoký)

    Serial.println("🎨 Kreslím menu...");

    for(int i=0; i<ACTION_COUNT; i++){
        int x = i * iconWidth;
        int y = menuY;

        // Pozadí ikony
        if(i == selectedAction) {
            tft.fillRect(x, y, iconWidth, iconHeight, TFT_BLUE);
        } else {
            tft.fillRect(x, y, iconWidth, iconHeight, TFT_DARKGREY);
        }

        // Rámeček
        tft.drawRect(x, y, iconWidth, iconHeight, TFT_WHITE);

        // Ikona soubor
        const char* iconFile = nullptr;
        switch(i){
            case 0: iconFile="/eatIcon.bmp"; break;
            case 1: iconFile="/sleepIcon.bmp"; break;
            case 2: iconFile="/bathIcon.bmp"; break;
            case 3: iconFile="/gameIcon.bmp"; break;
            case 4: iconFile="/healIcon.bmp"; break;
            case 5: iconFile="/statusIcon.bmp"; break;
        }

        // Ikony jsou přesně 40x28, vykreslí se celé (bez průhlednosti)
        if(iconFile) {
            Serial.printf("   Kreslím ikonu %d: %s na pozici (%d,%d)\n", i, iconFile, x, y);
            bool success = drawBmp(iconFile, x, y);
            if(!success) {
                Serial.printf("   ❌ Ikona %d se nenačetla!\n", i);
                // Pokud se ikona nenačte, zobraz text
                tft.setTextSize(1);
                tft.setTextColor(TFT_WHITE, i == selectedAction ? TFT_BLUE : TFT_DARKGREY);
                tft.setCursor(x + 12, y + 10);
                tft.print(actionShort[i]);
            }
        }
    }
    
    Serial.println("✅ Menu vykresleno");
}

// === Loading screen ===
void showLoading() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(40,60);
    tft.println("Loading...");
}

// === Chyba ===
void showError(const char* message) {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(20,60);
    tft.println(message);
}

#endif