#ifndef PIECES_H
#define PIECES_H

#define WHITE_KING_INDEX 10
#define BLACK_KING_INDEX 11
#define WHITE_SHORT_CASTLE_INDEX 6
#define WHITE_LONG_CASTLE_INDEX 2
#define BLACK_SHORT_CASTLE_INDEX 62
#define BLACK_LONG_CASTLE_INDEX 58

#include "board.h"

#include <stdint.h>

typedef struct {
    uint64_t *pos_bb;
    char symbol;
    char color;
    int value;
} Piece;

Piece *get_pieces(void);

int calculate_possible_moves(int position);

uint64_t get_full_board(void);

uint64_t find_possible_moves(Square input_square, Piece *piece,
                             GameState *game_state);

Piece *find_piece_by_position(int position);

int is_bit_set(uint64_t bb, int position);

Piece *get_piece_bb(char piece);

#endif