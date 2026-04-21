#include "board.h"

#include "pieces.h"

#include <stdint.h>
#include <stdio.h>

void print_bitboard(uint64_t bitboard)
{
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint64_t mask = (uint64_t) 1 << sq;

            if (bitboard & mask)
                printf("x");
            else
                printf(".");
        }
        printf("\n");
    }
    printf("   --------\n");
    printf("   abcdefgh\n");
}

uint64_t get_full_board(BoardState board)
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
