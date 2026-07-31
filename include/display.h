#pragma once

#include <TFT_eSPI.h>
#include <SPI.h>
#include "logic.h"

#define BLOCK_SIZE 11
#define OFFSET_X 20
#define OFFSET_Y 10
#define UI_LEFT_MARGIN 160

extern TFT_eSPI tft;

/// @brief Renders game board and pieces
/// @param s Game state
void drawGame(State *s);

/// @brief Renders score, next piece, UI
/// @param s Game state
void drawUI(State *s);
