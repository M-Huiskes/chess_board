#ifndef BOARD_H
#define BOARD_H

#define SQUARE_SIZE 75
#define FILE_OFFSET 'a'
#define ROW_OFFSET '1'

typedef struct
{
    int file;
    int row;
} Square;

typedef struct
{
    int total_moves;
    char last_move[5];
    char last_moved_piece;
    int play_en_passant;
    int promote_pawn;
    char promote_to;
} GameState;

Square square_from_position(int position);

#endif