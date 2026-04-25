#ifndef PIECES_H
#define PIECES_H

#include <stdint.h>

const extern uint64_t START_WHITE_PAWNS;
const extern uint64_t START_BLACK_PAWNS;

const extern uint64_t START_WHITE_ROOKS;
const extern uint64_t START_BLACK_ROOKS;

const extern uint64_t START_WHITE_KNIGHTS;
const extern uint64_t START_BLACK_KNIGHTS;

const extern uint64_t START_WHITE_BISHOPS;
const extern uint64_t START_BLACK_BISHOPS;

const extern uint64_t START_WHITE_QUEEN;
const extern uint64_t START_BLACK_QUEEN;

const extern uint64_t START_WHITE_KING;
const extern uint64_t START_BLACK_KING;

typedef struct {
    uint64_t pos_bb;
    char symbol;
    char color;
    int value;
} Piece;

void init_pieces(Piece team[6], char color);

#endif