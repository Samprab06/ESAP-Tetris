#pragma once

#include <SPI.h>
#include "ESP_I2S.h"
#include "sdcard.h"

const size_t CHUNK_SIZE = 1024;

extern I2SClass I2S;

extern I2SClass I2S;
extern File audioFile;
extern uint8_t chunkBuffer[CHUNK_SIZE];

/// @brief Plays WAV audio from SD card buffer
void playWav();