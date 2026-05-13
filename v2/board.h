#ifndef BOARD_H
#define BOARD_H

#include "pieces.h"

typedef struct {
    TeamState white;
    TeamState black;
} BoardState;

typedef struct {
    int file;
    int row;
} Square;

typedef enum {
    PLAYING = 0,
    CHECK_MATE = 1,
    STALE_MATE = 2,
} GameEnd;

BoardState init_board(void);
int position_from_square(Square *input_square);
uint64_t get_full_bit_board(BoardState *board);
Piece *get_piece_by_index(int index, BoardState *board);
Piece *get_piece_by_square(Square *input_square, BoardState *board);
Piece *get_piece_by_position(int position, BoardState *board);
Square square_from_position(int position);
void make_move(GameState *game_state);
char color_to_move(GameState *game_state);
int is_enemy(char piece_color, int position, BoardState *board);
void print_bitboard(uint64_t bitboard);
int has_game_ended(GameState *game_state);

#endif