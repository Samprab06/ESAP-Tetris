# ESP32 Tetris game logic 
As tested on esp32 devkit v1

## hardwre wiring
spi display - MOSI: 23 SCK:18 CS:5 LED:3V3 RST:4 DC:2 (Change tft_espi config.h to represent the correct CS pin)
Buttons (active high thru 1k resistor)
left:36
right:39
down:34 
rotate:35