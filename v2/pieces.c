#include "pieces.h"

#include "board.h"

#include <stdint.h>

const uint64_t START_WHITE_PAWNS = 0x000000000000FF00ULL;
const uint64_t START_BLACK_PAWNS = 0x00FF000000000000ULL;
const uint64_t START_WHITE_ROOKS = 0x0000000000000081ULL;
const uint64_t START_BLACK_ROOKS = 0x8100000000000000ULL;
const uint64_t START_WHITE_KNIGHTS = 0x0000000000000042ULL;
const uint64_t START_BLACK_KNIGHTS = 0x4200000000000000ULL;
const uint64_t START_WHITE_BISHOPS = 0x0000000000000024ULL;
const uint64_t START_BLACK_BISHOPS = 0x2400000000000000ULL;
const uint64_t START_WHITE_QUEEN = 0x0000000000000008ULL;
const uint64_t START_BLACK_QUEEN = 0x0800000000000000ULL;
const uint64_t START_WHITE_KING = 0x0000000000000010ULL;
const uint64_t START_BLACK_KING = 0x1000000000000000ULL;

// uint64_t find_possible_moves(Piece *piece, BoardState board) {}

void init_pieces(Piece team[6], char color)
{
    uint64_t start_positions[] = {
        color == 'w' ? START_WHITE_PAWNS : START_BLACK_PAWNS,
        color == 'w' ? START_WHITE_ROOKS : START_BLACK_ROOKS,
        color == 'w' ? START_WHITE_KNIGHTS : START_BLACK_KNIGHTS,
        color == 'w' ? START_WHITE_BISHOPS : START_BLACK_BISHOPS,
        color == 'w' ? START_WHITE_KING : START_BLACK_KING,
        color == 'w' ? START_WHITE_QUEEN : START_BLACK_QUEEN,
    };
    char *symbols;
    if (color == 'w') {
        symbols = "PRNBQK";
    } else {
        symbols = "prnbqk";
    }
    int values[] = {1, 3, 3, 5, 9, 0};

    for (int i = 0; i < 6; i++) {
        team[i] = (Piece){
            .color = color,
            .pos_bb = start_positions[i],
            .symbol = symbols[i],
            .value = values[i],
        };
    }
}