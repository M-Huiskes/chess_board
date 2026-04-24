#ifndef BOARD_H
#define BOARD_H

#include "pieces.h"

typedef struct {
    Piece white[6];
    Piece black[6];
} BoardState;

typedef struct {
    int file;
    int row;
} Square;

uint64_t get_full_bit_board(BoardState board);

#endif