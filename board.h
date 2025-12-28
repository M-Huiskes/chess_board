#ifndef BOARD_H
#define BOARD_H

#define SQUARE_SIZE 75
#define FILE_OFFSET 'a'
#define ROW_OFFSET '1'

#include <stdint.h>

typedef struct {
    int file;
    int row;
} Square;

typedef struct {
    int short_castle_allowed;
    int long_castle_allowed;
} TeamState;

typedef struct {
    int total_moves;
    char last_move[5];
    char last_moved_piece;
    int play_en_passant;
    int promote_pawn;
    char promote_to;
    int is_check;
    char last_captured_piece;
    char castle_played;
    TeamState *white_state;
    TeamState *black_state;
} GameState;

Square square_from_position(int position);

int get_position(int file, int row);

char color_to_move(GameState *game_state);

void set_bit(uint64_t *piece_bb, int position);

void print_bitboard(uint64_t possible_moves);

#endif