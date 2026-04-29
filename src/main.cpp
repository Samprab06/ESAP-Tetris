#include "main.hpp"

int prev_disp[BOARD_H][BOARD_W] = {0};
bool force_redraw = true;
bool gameOverDrawn = false;

int in_bounds(int x, int y)
{
    return 0 <= x && x < BOARD_W && 0 <= y;
}

int can_place(Grid *g, Piece *p)
{
    int i = 4 * p->type + p->rot;
    int px = p->x, py = p->y;
    rep(y, 0, 4) rep(x, 0, 4) if ((PIECES[i] >> (4 * y + x)) & 1)
    {
        if (!in_bounds(px + x, py + y))
            return 0;
        if (py + y >= BOARD_H)
            continue;
        if (g->c[py + y][px + x] > 0)
            return 0;
    }
    return 1;
}

void lock_piece(Grid *g, Piece *p)
{
    int i = 4 * p->type + p->rot;
    int px = p->x, py = p->y;
    rep(y, 0, 4) rep(x, 0, 4) if ((PIECES[i] >> (4 * y + x)) & 1)
    {
        if (py + y >= BOARD_H)
            continue;
        g->c[py + y][px + x] = PCOLOR[p->type];
    }
}

int clear_lines(Grid *g)
{
    int lc = 0;
    rep(y, 0, BOARD_H)
    {
        int all = 1;
        rep(x, 0, BOARD_W) if (g->c[y][x] == 0)
        {
            all = 0;
            break;
        }
        if (all)
        {
            lc += 1;
            continue;
        }
        rep(x, 0, BOARD_W) g->c[y - lc][x] = g->c[y][x];
    }
    rep(y, BOARD_H - lc, BOARD_H) rep(x, 0, BOARD_W) g->c[y][x] = 0;
    return lc;
}

void rot_piece(Piece *p, int d) { p->rot = (p->rot + 4 + d) % 4; }
void mv_piece(Piece *p, int dx, int dy)
{
    p->x += dx;
    p->y += dy;
}
void update_gravity_rst(State *state) { state->gravity_rst = LEVEL_SPEED[state->lines / 10]; }

// new piece generation, was broken for some reason
void new_piece(State *state)
{
    state->cur = Piece{3, 18, 0, state->nxt_type};
    int t1 = random(7);
    if (state->nxt_type == t1)
    {
        t1 = random(7);
    }
    state->nxt_type = t1;
}

// initialisation
void init_state(State *state)
{
    state->score = 0;
    state->lines = 0;
    rep(y, 0, BOARD_H) rep(x, 0, BOARD_W)(state->grid).c[y][x] = 0;
    state->gravity_rst = LEVEL_SPEED[0];
    state->gravity = state->gravity_rst;
    state->alive = 1;

    prevScore = -1;
    prevNextType = -1;
    force_redraw = true;
    gameOverDrawn = false;

    state->nxt_type = random(7);
    new_piece(state);
}

void update(State *state, const Input *input)
{
    if (!state->alive)
        return;
    Piece *p = &state->cur;
    Grid *g = &state->grid;

    if (input->L)
    {
        mv_piece(p, -1, 0);
        if (!can_place(g, p))
            mv_piece(p, +1, 0);
    }
    else if (input->R)
    {
        mv_piece(p, +1, 0);
        if (!can_place(g, p))
            mv_piece(p, -1, 0);
    }

    if (input->Z)
    {
        rot_piece(p, +1);
        if (!can_place(g, p))
        {
            mv_piece(p, 1, 0); // try right wall-kick
            if (!can_place(g, p))
            {
                mv_piece(p, -2, 0); // try left wall-kick
                if (!can_place(g, p))
                {
                    mv_piece(p, 1, 0); // revert move
                    rot_piece(p, -1);  // revert rot
                }
            }
        }
    }
    else if (input->X)
    {
        rot_piece(p, -1);
        if (!can_place(g, p))
        {
            mv_piece(p, 1, 0);
            if (!can_place(g, p))
            {
                mv_piece(p, -2, 0);
                if (!can_place(g, p))
                {
                    mv_piece(p, 1, 0);
                    rot_piece(p, +1);
                }
            }
        }
    }

    if (input->D)
        state->gravity = min(state->gravity, 2);

    state->gravity--;
    if (state->gravity == 0)
    {
        state->gravity = state->gravity_rst;
        mv_piece(p, 0, -1);
        if (!can_place(g, p))
        {
            mv_piece(p, 0, +1);
            if (!can_place(g, p))
            {
                state->alive = 0;
            } // end game

            lock_piece(g, p);
            int lc = clear_lines(g);
            if (lc > 0)
            {
                state->lines += lc;
                state->score += PTS[lc] * (1 + (state->lines / 10));
                state->gravity_rst = LEVEL_SPEED[state->lines / 10];
                state->gravity = min(state->gravity, state->gravity_rst);
            }
            new_piece(state);
        }
    }
}

// Display scaling for 480x320 (Landscape)
const int DRAW_BLOCK_SIZE = 15; // Scaled down so 20 blocks fit in 320px height
const int DRAW_OFFSET_X = 160;  // Centered horizontally on 480px width
const int DRAW_OFFSET_Y = 10;   // Small top margin

void drawGame(State *s)
{
    Grid *g = &s->grid;
    Piece *p = &s->cur;

    for (int y = BOARD_H - 1; y >= 0; y--)
    {
        for (int x = 0; x < BOARD_W; x++)
        {
            int c = g->c[y][x];

            int dx = x - p->x;
            int dy = y - p->y;
            if (0 <= dx && dx < 4 && 0 <= dy && dy < 4)
            {
                int i = 4 * p->type + p->rot;
                if ((PIECES[i] >> (4 * dy + dx)) & 1)
                {
                    c = PCOLOR[p->type];
                }
            }

            // Only redraw the block if its color has changed
            if (!force_redraw && c == prev_disp[y][x])
                continue;
            
            prev_disp[y][x] = c;

            uint16_t blockColor = TFT_BLACK;
            if (c == 1)
                blockColor = TFT_RED;
            else if (c == 2)
                blockColor = TFT_GREEN;
            else if (c == 3)
                blockColor = TFT_BLUE;

            int screenX = DRAW_OFFSET_X + (x * DRAW_BLOCK_SIZE);
            int screenY = DRAW_OFFSET_Y + ((BOARD_H - 1 - y) * DRAW_BLOCK_SIZE);

            tft.fillRect(screenX, screenY, DRAW_BLOCK_SIZE - 1, DRAW_BLOCK_SIZE - 1, blockColor);
        }
    }
    force_redraw = false;
}
void drawUI(State *s)
{

    tft.drawRect(DRAW_OFFSET_X - 1, DRAW_OFFSET_Y - 1, (BOARD_W * DRAW_BLOCK_SIZE) + 2, (BOARD_H * DRAW_BLOCK_SIZE) + 2, TFT_WHITE);
    int uiX = 360; // Push UI elements further right to fit the landscape display

    if (s->nxt_type != prevNextType)
    {
        tft.fillRect(uiX, 20, 85, 130, TFT_BLACK); // Increased clear area for larger blocks
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("NEXT", uiX, 20, 2);

        int type = s->nxt_type;
        uint16_t color = (PCOLOR[type] == 1) ? TFT_RED : (PCOLOR[type] == 2 ? TFT_GREEN : TFT_BLUE);

        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                if ((PIECES[4 * type] >> (4 * (3 - y) + x)) & 1)
                {
                        tft.fillRect(uiX + (x * 20), 50 + (y * 20), 19, 19, color); // Scaled up next block size
                }
            }
        }
        prevNextType = s->nxt_type;
    }

    // 3. Score (Now clearly visible at bottom right)
    if (s->score != prevScore)
    {
        tft.fillRect(uiX, 200, 80, 80, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("SCORE", uiX, 200, 2);
        tft.drawNumber(s->score, uiX, 220, 4); // Larger font for score
        prevScore = s->score;
    }
}

void setup()
{
    Serial.begin(115200);

    tft.init();
    // tft.setRotation(0); // Vertical
    tft.setRotation(1);    // Horizontal
    tft.fillScreen(TFT_BLACK);

    pinMode(PIN_LEFT, INPUT);
    pinMode(PIN_RIGHT, INPUT);
    pinMode(PIN_DOWN, INPUT);
    pinMode(PIN_ROT, INPUT);
    pinMode(PIN_START, INPUT);

    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card Mount Failed. Audio disabled.");
    } else {
        Serial.println("SD Card Mount Success!");
        out = new AudioOutputI2S();
        out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
        
        file_raw = new AudioFileSourceSD("/tetris.mp3");
        file = new AudioFileSourceBuffer(file_raw, 16384); // Read 16KB of MP3 into RAM to prevent SPI starvation
        mp3 = new AudioGeneratorMP3();
    }

    randomSeed(esp_random());
    init_state(&gameState);

    gameState.paused = true;
    drawUI(&gameState);

    // --- Dedicated Audio Task on Core 0 ---
    xTaskCreatePinnedToCore(
        [](void *parameter) {
            while (true) {
                bool shouldPlay = (!gameState.paused && gameState.alive);
                if (mp3) {
                    if (shouldPlay) {
                        if (!mp3->isRunning()) {
                            mp3->begin(file, out);
                        }
                        if (!mp3->loop()) {
                            // Reached end of file, loop track
                            mp3->stop();
                            file->seek(0, SEEK_SET); 
                        }
                    } else {
                        if (mp3->isRunning()) {
                            mp3->stop(); // Pause track
                        }
                    }
                }
                // Yield 1ms to prevent Watchdog crash and let Core 0 do other things
                vTaskDelay(pdMS_TO_TICKS(1)); 
            }
        },
        "AudioTask",
        4096, // 4KB stack space
        NULL,
        1,    // Priority
        NULL,
        0     // Pin to Core 0 (Arduino loop runs on Core 1)
    );
}

void loop()
{
    // --- Audio Processing (Needs continuous polling without delay) ---
    bool shouldPlay = (!gameState.paused && gameState.alive);
    if (mp3) {
        if (shouldPlay) {
            if (!mp3->isRunning()) {
                mp3->begin(file, out);
            }
            if (!mp3->loop()) {
                // Reached end of file, loop track
                mp3->stop();
                file->seek(0, SEEK_SET); 
            }
        } else {
            if (mp3->isRunning()) {
                mp3->stop(); // Pause track
            }
        }
    }

    unsigned long currentMillis = millis();
    static unsigned long lastFrame = 0;
    if (currentMillis - lastFrame < 16) return; // Non-blocking ~60 FPS limit
    lastFrame = currentMillis;

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
                tft.drawCentreString("PAUSED", 240, 160, 4); // Centered on 480x320
            }
            else
            {
                // Clear the "PAUSED" text when unpausing
                tft.fillRect(140, 130, 200, 60, TFT_BLACK);
                force_redraw = true; // Redraw blocks that were hidden by the text
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
        if (!gameOverDrawn) 
        {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawCentreString("GAME OVER", 240, 140, 4);
            tft.drawCentreString("Press START to Reset", 240, 180, 2);
            gameOverDrawn = true;
        }
    }

    drawUI(&gameState);
}