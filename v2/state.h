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
    uint16_t move;
    char captured_piece;
} MoveRecord;

typedef struct {
    MoveRecord *moves;
    int count;
    int capacity;
} MoveHistory;

typedef struct {
    int is_check;
    char check_by;
    int is_double_check;
    int position_check;
} CheckInfo;

typedef struct GameState {
    BoardState board;
    Piece *selected_piece;
    Square selected_square;
    int bit_position;
    MoveHistory move_history;
    char last_moved_piece;
    int en_passant_possible;
    char promote_to;
    int output_position;
    uint64_t possible_moves;
    int awaiting_promotion;
    CheckInfo check_info;
} GameState;

GameState init_game_state();
MoveRecord encode_move(int from, int to, int flags, char captured_piece);
int get_from(uint16_t move);
int get_to(uint16_t move);
int get_flags(uint16_t move);
void push_move(MoveHistory *history, MoveRecord move);

#endif