#include "display.h"

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

            uint16_t blockColor = TFT_BLACK;
            if (c == 1)
                blockColor = TFT_RED;
            else if (c == 2)
                blockColor = TFT_GREEN;
            else if (c == 3)
                blockColor = TFT_BLUE;

            int screenX = OFFSET_X + (x * BLOCK_SIZE);
            int screenY = OFFSET_Y + ((BOARD_H - 1 - y) * BLOCK_SIZE);

            tft.fillRect(screenX, screenY, BLOCK_SIZE - 1, BLOCK_SIZE - 1, blockColor);
        }
    }
}

void drawUI(State *s)
{

    tft.drawRect(OFFSET_X - 1, OFFSET_Y - 1, (BOARD_W * BLOCK_SIZE) + 2, (BOARD_H * BLOCK_SIZE) + 2, TFT_WHITE);
    int uiX = 180;

    if (s->nxt_type != prevNextType)
    {
        tft.fillRect(uiX, 20, 80, 70, TFT_BLACK);
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
                    tft.fillRect(uiX + (x * 10), 40 + (y * 10), 9, 9, color);
                }
            }
        }
        prevNextType = s->nxt_type;
    }

    // 3. Score (Now clearly visible at bottom right)
    if (s->score != prevScore)
    {
        tft.fillRect(uiX, 180, 100, 40, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("SCORE", uiX, 180, 2);
        tft.drawNumber(s->score, uiX, 195, 4); // Larger font for score
        prevScore = s->score;
    }
}