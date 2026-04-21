#ifndef PIECES_H
#define PIECES_H

#include <stdint.h>

extern uint64_t START_WHITE_PAWNS;
extern uint64_t START_BLACK_PAWNS;

extern uint64_t START_WHITE_ROOKS;
extern uint64_t START_BLACK_ROOKS;

extern uint64_t START_WHITE_KNIGHTS;
extern uint64_t START_BLACK_KNIGHTS;

extern uint64_t START_WHITE_BISHOPS;
extern uint64_t START_BLACK_BISHOPS;

extern uint64_t START_WHITE_QUEEN;
extern uint64_t START_BLACK_QUEEN;

extern uint64_t START_WHITE_KING;
extern uint64_t START_BLACK_KING;

typedef struct {
    uint64_t *pos_bb;
    char symbol;
    char color;
    int value;
} Piece;

#endif