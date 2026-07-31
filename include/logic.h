#pragma once

#include <Arduino.h>

#define BOARD_W 10
#define BOARD_H 20
#define rep(i, a, b) for (int i = (a); i < (b); i++)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

/// @brief Checks if coordinates are in bounds
int in_bounds(int x, int y);

/// @brief Checks if piece can be placed
int can_place(Grid *g, Piece *p);

/// @brief Locks piece position on grid
void lock_piece(Grid *g, Piece *p);

/// @brief Clears completed lines, returns count
int clear_lines(Grid *g);

/// @brief Rotates piece clockwise or counterclockwise
void rot_piece(Piece *p, int d);

/// @brief Moves piece by dx, dy offset
void mv_piece(Piece *p, int dx, int dy);

/// @brief Updates gravity reset based on level
void update_gravity_rst(State *state);

/// @brief Spawns new falling piece
void new_piece(State *state);

/// @brief Initializes game state to start
void init_state(State *state);

/// @brief Updates game state with input
void update(State *state, const Input *input);

/// @brief Renders game board and pieces
void drawGame(State *s);

/// @brief Renders score, next piece, UI
void drawUI(State *s);

extern State gameState;
extern Input gameInput;

extern bool lastRot;
extern unsigned long lPressTime;
extern bool lDasActive;
extern unsigned long rPressTime;
extern bool rDasActive;
extern bool lastStart;

extern int prevScore;
extern int prevNextType;

/// @brief Checks if coordinates are in bounds
/// @param x X coordinate
/// @param y Y coordinate
/// @return True if in bounds
int in_bounds(int x, int y);

/// @brief Checks if piece can be placed
/// @param g Game grid
/// @param p Piece to check
/// @return True if can place
int can_place(Grid *g, Piece *p);

/// @brief Locks piece position on grid
/// @param g Game grid
/// @param p Piece to lock
void lock_piece(Grid *g, Piece *p);

/// @brief Clears completed lines, returns count
/// @param g Game grid
/// @return Number of lines cleared
int clear_lines(Grid *g);

/// @brief Rotates piece clockwise or counterclockwise
/// @param p Piece to rotate
/// @param d Direction (1 or -1)
void rot_piece(Piece *p, int d);

/// @brief Moves piece by dx, dy offset
/// @param p Piece to move
/// @param dx X offset
/// @param dy Y offset
void mv_piece(Piece *p, int dx, int dy);

/// @brief Updates gravity reset based on level
/// @param state Game state
void update_gravity_rst(State *state);

/// @brief Spawns new falling piece
/// @param state Game state
void new_piece(State *state);

/// @brief Initializes game state to start
/// @param state Game state
void init_state(State *state);

/// @brief Updates game state with input
/// @param state Game state
/// @param input Player input
void update(State *state, const Input *input);