#ifndef STATE_H
#define STATE_H

#include "board.h"
#include "computer.h"

#include <stdint.h>

typedef enum {
    QUIET = 0,
    DOUBLE_PAWN = 1,
    CASTLE_SHORT = 2,
    CASTLE_LONG = 3,
    CAPTURE = 4,
    EN_PASSANT = 5,
    PROMOTION = 6,
    PROMOTION_CAPTURE = 7,
    DISABLED_SHORT_CASTLE = 8,
    DISABLED_LONG_CASTLE = 9,
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
    ComputerMove computer_move;
} GameState;

typedef struct OldState {
    char last_moved_piece;
    int en_passant_possible;
    char promote_to;
    uint64_t possible_moves;
    int awaiting_promotion;
    CheckInfo check_info;
    int white_short_castle;
    int white_long_castle;
    int black_short_castle;
    int black_long_castle;
} OldState;

GameState init_game_state();
MoveRecord encode_move(int from, int to, int flags, char captured_piece);
OldState write_old_state(GameState *game_state);
int get_from(uint16_t move);
int get_to(uint16_t move);
int get_flags(uint16_t move);
void push_move(MoveHistory *history, MoveRecord move);

#endif