#include "display.h"
#include <Arduino.h>
#include "buttons.h" 

extern TFT_eSPI tft;
extern volatile int selectedAction;

// 1. funkce pro čtení pixelů
// BMP obrázky ukládají barvy jako 3 čísla (Modrá, Zelená, Červená)
// Tato funkce vezme data ze souboru a přeloží je pro displej
uint16_t readPixel(File &f, int bpp) {
    if (bpp >= 24) {
        uint8_t b = f.read();
        uint8_t g = f.read(); 
        uint8_t r = f.read();

        if (bpp == 32) f.read(); // Přeskočit Alpha kanál (pokud je obrázek 32-bitový)
        return tft.color565(r, g, b); // Převod na formát pro displej

    } else if (bpp == 16) {
        // Pokud už je obrázek 16-bitový, jen spojíme dva bajty dohromady
        return f.read() | (f.read() << 8);
    }
    return 0;
}

// 2. vykreslení celého BMP obrázku
bool drawBmp(const char *filename, int16_t x, int16_t y) {
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false; //soubor neexistuje

    // Kontrola hlavičky BMP souboru
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; }

    // a) Čtení hlavičky bmp souboru
    // na pozici 10 je napsáno, kde začínají data obrázku
    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    
    // na pozici 18 je šířka a výška obrázku
    bmpFS.seek(18); int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
                    int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);

    // na pozici 28 je barevná hloubka (bits per pixel)                
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);

    // skok na začátek dat obrázku
    bmpFS.seek(seekOffset);

    // b) příprava paměti (buffer)
    // Aby bylo kreslení rychlé, načteme vždy celý jeden řádek pixelů do paměti RAM
    uint16_t *lineBuffer = (uint16_t*)malloc(w * 2); // každý pixel je 2 bajty
    if (!lineBuffer) { bmpFS.close(); return false; } 

    // c) čtení paddingu (výplň)
    // bmp ukládá řádky odspodu nahoru
    int padding = (bpp == 24) ? (4 - ((w * 3) % 4)) % 4 : 0;

    // d) smyčka vykreslování
    for (int row = h - 1; row >= 0; row--) {
        // načteme celý řádek pixelů do bufferu
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);
            // prohodíme byty pro správné zobrazení na TFT
            lineBuffer[col] = (color >> 8) | (color << 8); 
        }

        // přeskočíme padding
        for (int p = 0; p < padding; p++) bmpFS.read();
        // vykreslíme řádek z bufferu na displej
        tft.pushImage(x, y + row, w, 1, lineBuffer);
    }
    free(lineBuffer);

    bmpFS.close();
    return true;
}


// 3. vykreslení BMP obrázku s průhledností (magenta barva)
// používá se pro ikony s průhledným pozadím
bool drawBmpTransparent(const char *filename, int16_t x, int16_t y) {
    // oteviraní souboru stejně jako v minulé funkci
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false;
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; };

    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(18); int32_t w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
                    int32_t h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);
    bmpFS.seek(seekOffset);

    int padding = (bpp == 24) ? (4 - ((w * 3) % 4)) % 4 : 0;

    // magenta barva pro průhlednost
    const uint16_t MAGENTA = 0xF81F; 

    // konrola každého pixelu, pokud je magenta, nevykreslíme ho
    for (int row = h - 1; row >= 0; row--) {
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);

            // kreslíme jen pokud to není magenta
            if (color != MAGENTA) tft.drawPixel(x + col, y + row, color);
        }
        for (int p = 0; p < padding; p++) bmpFS.read();
    }
    bmpFS.close();
    return true;
}

// 4. vykreslení části BMP obrázku
// vykreslení pouze části obrázku definované šířkou a výškou (králíka)

bool drawBmpPartial(const char *filename, int16_t x, int16_t y, int16_t w, int16_t h) {
    File bmpFS = LittleFS.open(filename, "r");
    if (!bmpFS) return false;
    if (bmpFS.read() != 'B' || bmpFS.read() != 'M') { bmpFS.close(); return false; }

    bmpFS.seek(10); uint32_t seekOffset = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(18); int32_t bmp_w = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
                    int32_t bmp_h = bmpFS.read() | (bmpFS.read() << 8) | (bmpFS.read() << 16) | (bmpFS.read() << 24);
    bmpFS.seek(28); uint16_t bpp = bmpFS.read() | (bmpFS.read() << 8);

    uint16_t *lineBuffer = (uint16_t*)malloc(w * 2);
    if (!lineBuffer) { bmpFS.close(); return false; }
    int bytesPerPixel = bpp / 8;
    // Výpočet délky celého řádku v souboru (včetně paddingu)
    uint32_t rowSize = ((bmp_w * bytesPerPixel + 3) / 4) * 4;

    // procházení řádků obrázku, které chceme vykreslit - výška h
    for (int i = 0; i < h; i++) {
        int targetY = y + i; // Skutečná Y souřadnice na displeji
        // BMP je vzhůru nohama, musíme vypočítat, který řádek v souboru to je
        if (targetY >= 107) continue; 
        int targetRowBMP = bmp_h - 1 - targetY;
        if (targetRowBMP < 0 || targetRowBMP >= bmp_h) continue;

        // Skočíme na začátek dat + (číslo řádku * velikost řádku)
        bmpFS.seek(seekOffset + (targetRowBMP * rowSize));

        // Přeskočíme pixely před začátkem požadované části
        for (int p = 0; p < x; p++) {
             if(bpp >= 24) { bmpFS.read(); bmpFS.read(); bmpFS.read(); if(bpp==32) bmpFS.read(); }
             else { bmpFS.read(); bmpFS.read(); }
        }

        // čtení pixelů pro požadovanou šířku
        for (int col = 0; col < w; col++) {
            uint16_t color = readPixel(bmpFS, bpp);
            lineBuffer[col] = (color >> 8) | (color << 8);
        }

        // Vykreslení řádku na displej
        tft.pushImage(x, targetY, w, 1, lineBuffer);
    }
    free(lineBuffer);
    bmpFS.close();
    return true;
}

// inicializace displeje
void initDisplay() {
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH); //zapnutí displeje
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    //Serial.println("✅ Display inicializován");
}

void showLoading() {
    tft.fillScreen(TFT_BLACK); tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2); tft.setCursor(40, 60); tft.println("Loading...");
}
void showError(const char* message) {
    tft.fillScreen(TFT_RED); tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(2); tft.setCursor(20, 60); tft.println(message);
}

// 5. vykreslení menu s ikonami a rámečkem kolem vybrané ikony
void drawMenu() {
    int iconWidth = 40;
    int iconHeight = 28; 
    int menuY = 107; // pozice menu na obrazovce

    // barva rámečku kolem vybrané ikony
    uint16_t darkPink = tft.color565(200, 20, 100);

    for(int i = 0; i < ACTION_COUNT; i++) {
        int x = i * iconWidth; // každá ikona se posubedně o svou šířku
        int y = menuY;

        const char* iconFile = nullptr;
        switch(i) {
            case 0: iconFile = "/eatIcon.bmp"; break;
            case 1: iconFile = "/sleepIcon.bmp"; break;
            case 2: iconFile = "/bathIcon.bmp"; break;
            case 3: iconFile = "/gameIcon.bmp"; break;
            case 4: iconFile = "/healIcon.bmp"; break;
            case 5: iconFile = "/statusIcon.bmp"; break;
        }

        //vykreslení ikony
        if(iconFile) drawBmp(iconFile, x, y);

        // pokud je tato ikona vybraná, vykreslíme kolem ní rámeček
        if(i == selectedAction) {
            // Vnější rámeček
            tft.drawRect(x, y, iconWidth, iconHeight, darkPink);
            // Vnitřní rámeček 
            tft.drawRect(x+1, y+1, iconWidth-2, iconHeight-2, darkPink);
        }
    }
}