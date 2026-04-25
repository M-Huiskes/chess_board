#include "board.h"

#include "pieces.h"

#include <stdint.h>
#include <stdio.h>

Piece *get_piece_by_index(int index, BoardState *board)
{
    if (index < 6) {
        return &(board->white[index]);
    } else {
        return &(board->black[index - 6]);
    }
}

int position_from_square(Square input_square)
{
    return input_square.row * 8 + input_square.file;
}

uint64_t get_full_bit_board(BoardState board)
{
    uint64_t full_board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, &board);
        full_board |= *(piece->pos_bb);
    }
    return full_board;
}

Piece *get_piece_by_square(Square input_square, BoardState board)
{
    int position = position_from_square(input_square);

    if (position < 0 || position > 63) {
        return NULL;
    }

    uint64_t mask = (uint64_t) 1 << position;

    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, &board);
        if (*(piece->pos_bb) & mask) {
            return piece;
        }
    }

    return NULL;
}