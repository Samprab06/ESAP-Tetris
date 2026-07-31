#include "logic.h"

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
            rot_piece(p, -1);
    }
    else if (input->X)
    {
        rot_piece(p, -1);
        if (!can_place(g, p))
            rot_piece(p, +1);
    }

    if (input->D)
        state->gravity = MIN(state->gravity, 2);

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
                state->gravity = MIN(state->gravity, state->gravity_rst);
            }
            new_piece(state);
        }
    }
}