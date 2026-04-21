#ifndef BOARD_H
#define BOARD_H

#include "pieces.h"

typedef struct {
    Piece white[6];
    Piece black[6];
} BoardState;

void print_bitboard(uint64_t bitboard);
uint64_t get_full_board(BoardState board);

#endif