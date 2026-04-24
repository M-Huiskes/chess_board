#ifndef BOARD_H
#define BOARD_H

#include "pieces.h"

typedef struct {
    Piece white[6];
    Piece black[6];
    Piece *selected_piece;
} BoardState;

typedef struct {
    int row;
    int file;
} Square;

uint64_t get_full_bit_board(BoardState board);

#endif