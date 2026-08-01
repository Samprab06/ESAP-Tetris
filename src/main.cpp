#include "main.hpp"

#ifndef WOKWI_SIM
SPIClass hspi(HSPI);
#endif

#ifndef WOKWI_SIM
void audioTask(void *pvParameters)
{
    printf("Audio task started on core %d\n", xPortGetCoreID());

    // sd card read through hspi
    hspi.begin(14, 12, 13, 15);

    if (!SD.begin(15, hspi))
    {
        printf("Card Mount Failed\n");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        printf("No SD card attached\n");
        return;
    }

    // File path
    audioFile = SD.open("/tetris.wav");
    if (!audioFile || audioFile.isDirectory())
    {
        printf("Failed to open WAV file\n");
    }
    else
    {
        audioFile.seek(44);
        printf("File opened, ready to stream\n");
    }

    // i2s audio
    // still need to verify the pins
    I2S.setPins(26, 25, 22, -1, -1);

    if (!I2S.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO))
    {
        // Handle initialization failure
        vTaskDelete(NULL);
    }

    while (true)
    {
        playWav();
        vTaskDelay(1);
    }
}
#endif

void setup()
{
    Serial.begin(115200);

    // display
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    pinMode(PIN_LEFT, INPUT_PULLDOWN);
    pinMode(PIN_RIGHT, INPUT_PULLDOWN);
    pinMode(PIN_DOWN, INPUT_PULLDOWN);
    pinMode(PIN_ROT, INPUT_PULLDOWN);
    pinMode(PIN_START, INPUT_PULLDOWN);

    randomSeed(esp_random());
    init_state(&gameState);

    gameState.paused = true;
    drawUI(&gameState);

#ifndef WOKWI_SIM
    xTaskCreatePinnedToCore(
        audioTask,
        "AudioTask",
        10000,
        NULL,
        2,
        NULL,
        0);
#endif
}

void loop()
{
    unsigned long currentMillis = millis();
    bool currStart = (digitalRead(PIN_START) == ACTIVE_STATE);

    // --- Start/Pause/Restart Logic ---
    if (currStart && !lastStart)
    {
        if (!gameState.alive)
        {
            // Restart game if dead
            tft.fillScreen(TFT_BLACK);
            init_state(&gameState);
            gameState.paused = false;
        }
        else
        {
            // Toggle pause if alive
            gameState.paused = !gameState.paused;
            if (gameState.paused)
            {
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.drawCentreString("PAUSED", 160, 110, 4);
            }
            else
            {
                // Clear the "PAUSED" text when unpausing
                tft.fillRect(100, 100, 120, 40, TFT_BLACK);
            }
        }
    }
    lastStart = currStart;

    if (!gameState.paused && gameState.alive)
    {
        // --- DAS Input Logic (Left/Right) ---
        bool currL = (digitalRead(PIN_LEFT) == ACTIVE_STATE);
        bool currR = (digitalRead(PIN_RIGHT) == ACTIVE_STATE);

        // Left DAS
        if (currL)
        {
            if (lPressTime == 0)
            {
                gameInput.L = 1;
                lPressTime = currentMillis;
                lDasActive = false;
            }
            else if (!lDasActive && (currentMillis - lPressTime >= DAS_DELAY))
            {
                gameInput.L = 1;
                lPressTime = currentMillis;
                lDasActive = true;
            }
            else if (lDasActive && (currentMillis - lPressTime >= ARR_DELAY))
            {
                gameInput.L = 1;
                lPressTime = currentMillis;
            }
            else
            {
                gameInput.L = 0;
            }
        }
        else
        {
            gameInput.L = 0;
            lPressTime = 0;
            lDasActive = false;
        }

        // Right DAS
        if (currR)
        {
            if (rPressTime == 0)
            {
                gameInput.R = 1;
                rPressTime = currentMillis;
                rDasActive = false;
            }
            else if (!rDasActive && (currentMillis - rPressTime >= DAS_DELAY))
            {
                gameInput.R = 1;
                rPressTime = currentMillis;
                rDasActive = true;
            }
            else if (rDasActive && (currentMillis - rPressTime >= ARR_DELAY))
            {
                gameInput.R = 1;
                rPressTime = currentMillis;
            }
            else
            {
                gameInput.R = 0;
            }
        }
        else
        {
            gameInput.R = 0;
            rPressTime = 0;
            rDasActive = false;
        }

        // Rotate and Down
        bool currRot = (digitalRead(PIN_ROT) == ACTIVE_STATE);
        gameInput.Z = (currRot && !lastRot) ? 1 : 0;
        gameInput.D = (digitalRead(PIN_DOWN) == ACTIVE_STATE);
        lastRot = currRot;

        update(&gameState, &gameInput);
        drawGame(&gameState);
    }
    else if (!gameState.alive)
    {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("GAME OVER", 160, 100, 4);
        tft.drawCentreString("Press START to Reset", 160, 140, 2);
    }

    drawUI(&gameState);
    delay(16);
}