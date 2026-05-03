#ifndef BOARD_H
#define BOARD_H

#include "pieces.h"

typedef struct {
    Piece pieces[6];
    uint64_t attack_map;
    int short_castle_allowed;
    int long_castle_allowed;
} TeamState;

typedef struct {
    TeamState white;
    TeamState black;
} BoardState;

typedef struct {
    int file;
    int row;
} Square;

BoardState init_board(void);
int position_from_square(Square *input_square);
uint64_t get_full_bit_board(BoardState *board);
Piece *get_piece_by_index(int index, BoardState *board);
Piece *get_piece_by_square(Square *input_square, BoardState *board);
Piece *get_piece_by_position(int position, BoardState *board);
Square square_from_position(int position);
void make_move(GameState *game_state);
char color_to_move(GameState *game_state);

#endif