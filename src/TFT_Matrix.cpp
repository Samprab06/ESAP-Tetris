#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); 

#define BOARD_W 10
#define BOARD_H 20
#define rep(i,a,b) for(int i=(a);i<(b);i++)
#define min(a,b) ((a)<(b)?(a):(b))

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

//delay for how long to hold down button to trigger auto move 
#define DAS_DELAY 250   
#define ARR_DELAY 50    

const int LEVEL_SPEED[30] = {
        48,43,38,33,28,23,18,13,8,
        6,5,5,5,4,4,4,3,3,3,
        2,2,2,2,2,2,2,2,2,2,1
};
const int PTS[5] = {0,40,100,300,1200};
const int PCOLOR[7] = {1,1,2,2,3,1,3}; 

const int PIECES[28] = {
        0b0000000011110000, 0b0100010001000100, 0b0000000011110000, 0b0100010001000100, //I
        0b0000000001100110, 0b0000000001100110, 0b0000000001100110, 0b0000000001100110, //O
        0b0000000011101000, 0b0000010001000110, 0b0000001011100000, 0b0000110001000100, //J
        0b0000000011100010, 0b0000011001000100, 0b0000100011100000, 0b0000010001001100, //L
        0b0000000011000110, 0b0000010011001000, 0b0000000011000110, 0b0000010011001000, //S
        0b0000000011100100, 0b0000010001100100, 0b0000010011100000, 0b0000010011000100, //T
        0b0000000001101100, 0b0000100011000100, 0b0000000001101100, 0b0000100011000100  //Z
};

typedef struct{ int c[BOARD_H][BOARD_W]; } Grid;
typedef struct{ int x,y,rot,type; } Piece;
typedef struct {
    int score, lines;
    Grid grid;
    Piece cur;
    int nxt_type;
    int gravity_rst;
    int gravity;
    bool alive;
    bool paused; 
} State;

typedef struct { int L, R, D, Z, X, Start; } Input;


int in_bounds( int x, int y );
int can_place( Grid* g, Piece* p );
void lock_piece( Grid* g, Piece* p );
int clear_lines( Grid* g );
void rot_piece( Piece* p, int d );
void mv_piece( Piece* p, int dx, int dy );
void update_gravity_rst( State* state );
void new_piece( State* state );
void init_state( State* state );
void update( State* state, const Input* input );
void drawGame(State* s);
void drawUI(State* s);


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


int in_bounds( int x, int y ){
   return 0 <= x && x < BOARD_W && 0 <= y; 
}

int can_place( Grid* g, Piece* p ){
        int i = 4 * p->type + p->rot;
        int px = p->x, py = p->y;
        rep(y,0,4) rep(x,0,4) if( (PIECES[i] >> (4*y + x))&1 ){
                if( !in_bounds(px+x,py+y) ) return 0;
                if( py+y >= BOARD_H ) continue;
                if( g->c[py+y][px+x] > 0 ) return 0;
        }
        return 1;
}

void lock_piece( Grid* g, Piece* p ){
        int i = 4 * p->type + p->rot;
        int px = p->x, py = p->y;
        rep(y,0,4) rep(x,0,4) if( (PIECES[i] >> (4*y + x))&1 ){
                if( py+y >= BOARD_H ) continue;
                g->c[py+y][px+x] = PCOLOR[p->type];
        }
}

int clear_lines( Grid* g ){
        int lc = 0;
        rep(y,0,BOARD_H){
                int all = 1;
                rep(x,0,BOARD_W) if( g->c[y][x] == 0 ){ all = 0; break; }
                if( all ){ lc += 1; continue; }
                rep(x,0,BOARD_W) g->c[y-lc][x] = g->c[y][x];
        }
        rep(y,BOARD_H-lc,BOARD_H) rep(x,0,BOARD_W) g->c[y][x] = 0;
        return lc;
}

void rot_piece( Piece* p, int d ){ p->rot = (p->rot + 4 + d)%4; }
void mv_piece( Piece* p, int dx, int dy ){ p->x += dx; p->y += dy; }
void update_gravity_rst( State* state ){ state -> gravity_rst = LEVEL_SPEED[ state -> lines / 10 ]; }

//new piece generation, was broken for some reason 
void new_piece( State* state ){
        state->cur = Piece{3,18,0,state->nxt_type}; 
        int t1 = random(7);
        if( state->nxt_type == t1 ){
                t1 = random(7);
        }
        state->nxt_type = t1;
}

//initialisation
void init_state( State* state ){
        state->score = 0;
        state->lines = 0;
        rep(y,0,BOARD_H) rep(x,0,BOARD_W) (state->grid).c[y][x] = 0;
        state->gravity_rst = LEVEL_SPEED[0];
        state->gravity = state->gravity_rst;
        state->alive = 1;
        
        prevScore = -1;
        prevNextType = -1;

        state->nxt_type = random(7);
        new_piece(state); 
}


void update( State* state, const Input* input ){
        if( !state->alive ) return;
        Piece* p = &state->cur;
        Grid* g = &state->grid;
        
        if( input -> L ){
                mv_piece( p,-1,0 );
                if( !can_place(g,p) ) mv_piece( p,+1,0 );
        } else if( input -> R ){
                mv_piece( p,+1,0 );
                if( !can_place(g,p) ) mv_piece( p,-1,0 );
        }
        
        if( input -> Z ){
                rot_piece( p,+1 );
                if( !can_place(g,p) ) rot_piece( p,-1 );
        } else if( input -> X ){
                rot_piece( p,-1 );
                if( !can_place(g,p) ) rot_piece( p,+1 );
        }
        
        if( input -> D ) state->gravity = min(state->gravity,2);

        state->gravity--;
        if( state->gravity == 0 ){
                state->gravity = state->gravity_rst;
                mv_piece( p,0,-1 );
                if( !can_place(g,p) ){
                        mv_piece( p,0,+1 );
                        if( !can_place(g,p) ){ state->alive = 0; } //end game
                        
                        lock_piece(g,p);
                        int lc = clear_lines(g);
                        if( lc > 0 ){
                                state->lines += lc;
                                state->score += PTS[lc] * (1 + (state->lines/10));
                                state->gravity_rst = LEVEL_SPEED[state->lines/10];
                                state->gravity = min(state->gravity,state->gravity_rst);
                        }
                        new_piece(state);
                }
        }
}


void drawGame(State* s) {
    Grid* g = &s->grid;
    Piece* p = &s->cur;

    for (int y = BOARD_H - 1; y >= 0; y--) {
        for (int x = 0; x < BOARD_W; x++) {
            int c = g->c[y][x];

            int dx = x - p->x;
            int dy = y - p->y;
            if (0 <= dx && dx < 4 && 0 <= dy && dy < 4) {
                int i = 4 * p->type + p->rot;
                if ((PIECES[i] >> (4 * dy + dx)) & 1) {
                    c = PCOLOR[p->type];
                }
            }

            uint16_t blockColor = TFT_BLACK;
            if (c == 1) blockColor = TFT_RED;
            else if (c == 2) blockColor = TFT_GREEN;
            else if (c == 3) blockColor = TFT_BLUE;

            int screenX = OFFSET_X + (x * BLOCK_SIZE);
            int screenY = OFFSET_Y + ((BOARD_H - 1 - y) * BLOCK_SIZE);

            tft.fillRect(screenX, screenY, BLOCK_SIZE - 1, BLOCK_SIZE - 1, blockColor);
        }
    }
}
void drawUI(State* s) {
   
    tft.drawRect(OFFSET_X - 1, OFFSET_Y - 1, (BOARD_W * BLOCK_SIZE) + 2, (BOARD_H * BLOCK_SIZE) + 2, TFT_WHITE);
    int uiX = 180; 

    
    if (s->nxt_type != prevNextType) {
        tft.fillRect(uiX, 20, 80, 70, TFT_BLACK);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString("NEXT", uiX, 20, 2);

        int type = s->nxt_type;
        uint16_t color = (PCOLOR[type] == 1) ? TFT_RED : (PCOLOR[type] == 2 ? TFT_GREEN : TFT_BLUE);

        for(int y = 0; y < 4; y++) {
            for(int x = 0; x < 4; x++) {
                if((PIECES[4 * type] >> (4 * (3-y) + x)) & 1) { 
                    tft.fillRect(uiX + (x * 10), 40 + (y * 10), 9, 9, color);
                }
            }
        }
        prevNextType = s->nxt_type;
    }

    // 3. Score (Now clearly visible at bottom right)
    if (s->score != prevScore) {
        tft.fillRect(uiX, 180, 100, 40, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("SCORE", uiX, 180, 2);
        tft.drawNumber(s->score, uiX, 195, 4); // Larger font for score
        prevScore = s->score;
    }
}
void setup() {
    Serial.begin(115200);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    pinMode(PIN_LEFT, INPUT);
    pinMode(PIN_RIGHT, INPUT);
    pinMode(PIN_DOWN, INPUT);
    pinMode(PIN_ROT, INPUT);
    pinMode(PIN_START, INPUT); 

    randomSeed(esp_random());
    init_state(&gameState);
    
    gameState.paused = true;
    drawUI(&gameState);
}
void loop() {
    unsigned long currentMillis = millis();
    bool currStart = (digitalRead(PIN_START) == ACTIVE_STATE);

    // --- Start/Pause/Restart Logic ---
    if (currStart && !lastStart) {
        if (!gameState.alive) {
            // Restart game if dead
            tft.fillScreen(TFT_BLACK);
            init_state(&gameState);
            gameState.paused = false;
        } else {
            // Toggle pause if alive
            gameState.paused = !gameState.paused;
            if (gameState.paused) {
                tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                tft.drawCentreString("PAUSED", 160, 110, 4);
            } else {
                // Clear the "PAUSED" text when unpausing
                tft.fillRect(100, 100, 120, 40, TFT_BLACK);
            }
        }
    }
    lastStart = currStart;

    if (!gameState.paused && gameState.alive) {
        // --- DAS Input Logic (Left/Right) ---
        bool currL = (digitalRead(PIN_LEFT) == ACTIVE_STATE);
        bool currR = (digitalRead(PIN_RIGHT) == ACTIVE_STATE);
        
        // Left DAS
        if (currL) {
            if (lPressTime == 0) { gameInput.L = 1; lPressTime = currentMillis; lDasActive = false; }
            else if (!lDasActive && (currentMillis - lPressTime >= DAS_DELAY)) { gameInput.L = 1; lPressTime = currentMillis; lDasActive = true; }
            else if (lDasActive && (currentMillis - lPressTime >= ARR_DELAY)) { gameInput.L = 1; lPressTime = currentMillis; }
            else { gameInput.L = 0; }
        } else { gameInput.L = 0; lPressTime = 0; lDasActive = false; }

        // Right DAS
        if (currR) {
            if (rPressTime == 0) { gameInput.R = 1; rPressTime = currentMillis; rDasActive = false; }
            else if (!rDasActive && (currentMillis - rPressTime >= DAS_DELAY)) { gameInput.R = 1; rPressTime = currentMillis; rDasActive = true; }
            else if (rDasActive && (currentMillis - rPressTime >= ARR_DELAY)) { gameInput.R = 1; rPressTime = currentMillis; }
            else { gameInput.R = 0; }
        } else { gameInput.R = 0; rPressTime = 0; rDasActive = false; }

        // Rotate and Down
        bool currRot = (digitalRead(PIN_ROT) == ACTIVE_STATE);
        gameInput.Z = (currRot && !lastRot) ? 1 : 0;
        gameInput.D = (digitalRead(PIN_DOWN) == ACTIVE_STATE);
        lastRot = currRot;

        update(&gameState, &gameInput);
        drawGame(&gameState);
    } 
    else if (!gameState.alive) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("GAME OVER", 160, 100, 4);
        tft.drawCentreString("Press START to Reset", 160, 140, 2);
    }

    drawUI(&gameState);
    delay(16);
}