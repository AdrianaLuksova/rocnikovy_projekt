#include <TFT_eSPI.h>
#include <SPIFFS.h>
#include <FS.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke library

// Forward declarations
void drawBmp(const char *filename, int16_t x, int16_t y);
uint16_t read16(File &f);
uint32_t read32(File &f);

void setup() {
  Serial.begin(115200);

  // Initialize TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    while (1);
  }

  // Draw the image
  drawBmp("/image.bmp", 0, 0);
}

void loop() {}

//-----------------------------------------
// Draw BMP from SPIFFS
//-----------------------------------------
void drawBmp(const char *filename, int16_t x, int16_t y) {
  if ((x >= tft.width()) || (y >= tft.height())) return;

  File bmpFS = SPIFFS.open(filename, "r");
  if (!bmpFS) {
    Serial.println("File not found");
    return;
  }

  uint16_t w, h;
  uint8_t r, g, b;
  uint32_t seekOffset;
  uint16_t rowSize;
  bool flip = true;

  if (read16(bmpFS) == 0x4D42) { // BMP signature
    read32(bmpFS); // File size
    read32(bmpFS); // Creator bytes
    seekOffset = read32(bmpFS); // Start of image data
    read32(bmpFS); // Header size
    w = read32(bmpFS);
    h = read32(bmpFS);
    if (read16(bmpFS) == 1) { // # planes
      uint16_t depth = read16(bmpFS); // Bits per pixel
      if (depth == 24 && read32(bmpFS) == 0) { // 24-bit, uncompressed
        rowSize = (w * 3 + 3) & ~3;
        if (h < 0) {
          h = -h;
          flip = false;
        }

        tft.startWrite();
        bmpFS.seek(seekOffset);
        for (int row = 0; row < h; row++) {
          uint8_t sdbuffer[3 * w];
          int rowPos = flip ? (seekOffset + (h - 1 - row) * rowSize)
                            : (seekOffset + row * rowSize);
          bmpFS.seek(rowPos);
          bmpFS.read(sdbuffer, sizeof(sdbuffer));

          tft.setAddrWindow(x, y + row, w, 1);
          for (int col = 0; col < w; col++) {
            b = sdbuffer[col * 3];
            g = sdbuffer[col * 3 + 1];
            r = sdbuffer[col * 3 + 2];
            tft.pushColor(tft.color565(r, g, b));
          }
        }
        tft.endWrite();
      }
    }
  }
  bmpFS.close();
  Serial.println("BMP Loaded!");
}

//-----------------------------------------
uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read();
  return result;
}
