#include <TFT_eSPI.h>
#include <SPI.h>
#include "pet.h" //soubor s polem dat obrazku
TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.init();
tft.setRotation(0);
tft.fillScreen(TFT_BLACK);
tft.pushImage(0,0,135,240,pet); //souradnice
}

void loop(){
  
}
