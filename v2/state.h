#ifndef STATE_H
#define STATE_H

#include "board.h"

typedef struct {
    BoardState board;
    Piece *selected_piece;
    Square selected_square;
    int bit_position;
} GameState;

GameState init_game_state();

#endif