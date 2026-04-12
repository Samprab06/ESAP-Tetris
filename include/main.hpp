// #include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ESP_I2S.h>
// #include <ESP_NOW.h>
#include "sdcard.h"

TFT_eSPI tft = TFT_eSPI();

#define BOARD_W 10
#define BOARD_H 20
#define rep(i, a, b) for (int i = (a); i < (b); i++)
#define min(a, b) ((a) < (b) ? (a) : (b))

// pin Definitions
#define PIN_DOWN 34
#define PIN_ROT 35
#define PIN_LEFT 36
#define PIN_RIGHT 39
#define PIN_START 32
#define ACTIVE_STATE HIGH

#define BLOCK_SIZE 11
#define OFFSET_X 20
#define OFFSET_Y 10
#define UI_LEFT_MARGIN 160

// delay for how long to hold down button to trigger auto move
#define DAS_DELAY 250
#define ARR_DELAY 50

const int LEVEL_SPEED[30] = {
    48, 43, 38, 33, 28, 23, 18, 13, 8,
    6, 5, 5, 5, 4, 4, 4, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1};
const int PTS[5] = {0, 40, 100, 300, 1200};
const int PCOLOR[7] = {1, 1, 2, 2, 3, 1, 3};

const int PIECES[28] = {
    0b0000000011110000, 0b0100010001000100, 0b0000000011110000, 0b0100010001000100, // I
    0b0000000001100110, 0b0000000001100110, 0b0000000001100110, 0b0000000001100110, // O
    0b0000000011101000, 0b0000010001000110, 0b0000001011100000, 0b0000110001000100, // J
    0b0000000011100010, 0b0000011001000100, 0b0000100011100000, 0b0000010001001100, // L
    0b0000000011000110, 0b0000010011001000, 0b0000000011000110, 0b0000010011001000, // S
    0b0000000011100100, 0b0000010001100100, 0b0000010011100000, 0b0000010011000100, // T
    0b0000000001101100, 0b0000100011000100, 0b0000000001101100, 0b0000100011000100  // Z
};

typedef struct
{
    int c[BOARD_H][BOARD_W];
} Grid;
typedef struct
{
    int x, y, rot, type;
} Piece;
typedef struct
{
    int score, lines;
    Grid grid;
    Piece cur;
    int nxt_type;
    int gravity_rst;
    int gravity;
    bool alive;
    bool paused;
} State;

typedef struct
{
    int L, R, D, Z, X, Start;
} Input;

int in_bounds(int x, int y);
int can_place(Grid *g, Piece *p);
void lock_piece(Grid *g, Piece *p);
int clear_lines(Grid *g);
void rot_piece(Piece *p, int d);
void mv_piece(Piece *p, int dx, int dy);
void update_gravity_rst(State *state);
void new_piece(State *state);
void init_state(State *state);
void update(State *state, const Input *input);
void drawGame(State *s);
void drawUI(State *s);

State gameState;
Input gameInput;

bool lastRot = false;
unsigned long lPressTime = 0;
bool lDasActive = false;
unsigned long rPressTime = 0;
bool rDasActive = false;
bool lastStart = false;

int prevScore = -1;
int prevNextType = -1;