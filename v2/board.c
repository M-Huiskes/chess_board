#include "board.h"

#include "pieces.h"

#include <stdint.h>
#include <stdio.h>

uint64_t get_full_bit_board(BoardState board)
{
    uint64_t full_board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
        if (i < 6) {
            full_board |= *(board.white[i].pos_bb);
        } else {
            full_board |= *(board.black[i - 6].pos_bb);
        }
    }
    return full_board;
}
