#include "logic.h"
#include "display.h"
#include "audio.h"

// Define global state and input
State gameState;
Input gameInput;

// Define button state variables
bool lastRot = false;
unsigned long lPressTime = 0;
bool lDasActive = false;
unsigned long rPressTime = 0;
bool rDasActive = false;
bool lastStart = false;

// Define UI state variables
int prevScore = -1;
int prevNextType = -1;

// Define display object
TFT_eSPI tft = TFT_eSPI();

// Define audio buffer variables
I2SClass I2S;
File audioFile;
uint8_t chunkBuffer[CHUNK_SIZE];
