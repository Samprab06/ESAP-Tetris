#pragma once

#include <SPI.h>
#include "ESP_I2S.h"
#include "sdcard.h"
#include "display.h"
#include "logic.h"
#include "audio.h"
#include "globals.hpp"
// #include <ESP_NOW.h>

// pin Definitions
#define PIN_DOWN 34
#define PIN_ROT 35
#define PIN_LEFT 36
#define PIN_RIGHT 39
#define PIN_START 32
#define ACTIVE_STATE HIGH

// delay for how long to hold down button to trigger auto move
#define DAS_DELAY 250
#define ARR_DELAY 50
