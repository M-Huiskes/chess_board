#ifndef STATE_H
#define STATE_H

#include "board.h"

#include <stdint.h>

typedef enum {
    QUIET = 0,
    DOUBLE_PAWN = 1,
    CASTLE_KING = 2,
    CASTLE_QUEEN = 3,
    CAPTURE = 4,
    EN_PASSANT = 5,
    PROMO_KNIGHT = 8,
    PROMO_BISHOP = 9,
    PROMO_ROOK = 10,
    PROMO_QUEEN = 11,
    PROMO_CAPTURE_KNIGHT = 12,
    PROMO_CAPTURE_BISHOP = 13,
    PROMO_CAPTURE_ROOK = 14,
    PROMO_CAPTURE_QUEEN = 15,
} MoveFlag;

typedef struct {
    uint16_t *moves;
    int count;
    int capacity;
} MoveHistory;

typedef struct GameState {
    BoardState board;
    Piece *selected_piece;
    Square selected_square;
    int bit_position;
    MoveHistory move_history;
} GameState;

GameState init_game_state();

#endif