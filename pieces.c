#include "pieces.h"

#include "board.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t WHITE_PAWNS = 0x000000000000FF00ULL;
uint64_t BLACK_PAWNS = 0x00FF000000000000ULL;

uint64_t WHITE_ROOKS = 0x0000000000000081ULL;
uint64_t BLACK_ROOKS = 0x8100000000000000ULL;

uint64_t WHITE_KNIGHTS = 0x0000000000000042ULL;
uint64_t BLACK_KNIGHTS = 0x4200000000000000ULL;

uint64_t WHITE_BISHOPS = 0x0000000000000024ULL;
uint64_t BLACK_BISHOPS = 0x2400000000000000ULL;

uint64_t WHITE_QUEEN = 0x0000000000000008ULL;
uint64_t BLACK_QUEEN = 0x0800000000000000ULL;

uint64_t WHITE_KING = 0x0000000000000010ULL;
uint64_t BLACK_KING = 0x1000000000000000ULL;

Piece pieces[12] = {
    {&WHITE_PAWNS, 'P', 'w', 1},
    {&BLACK_PAWNS, 'p', 'b', 1},
    {&WHITE_ROOKS, 'R', 'w', 5},
    {&BLACK_ROOKS, 'r', 'b', 5},
    {&WHITE_KNIGHTS, 'N', 'w', 3},
    {&BLACK_KNIGHTS, 'n', 'b', 3},
    {&WHITE_BISHOPS, 'B', 'w', 3},
    {&BLACK_BISHOPS, 'b', 'b', 3},
    {&WHITE_QUEEN, 'Q', 'w', 9},
    {&BLACK_QUEEN, 'q', 'b', 9},
    // Value of zero for king may have to be changed for minimax algo
    {&WHITE_KING, 'K', 'w', 0},
    {&BLACK_KING, 'k', 'b', 0},
};

int calculate_possible_moves(int position)
{
    return position + 8;
}

uint64_t get_full_board(void)
{
    uint64_t board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
        board |= *(pieces[i].pos_bb);
    }
    return board;
}

Piece *get_pieces(void)
{
    return pieces;
}

int is_bit_set(uint64_t bb, int position)
{
    uint64_t mask = (uint64_t) 1 << position;
    return (bb & mask) ? 1 : 0;
}

int is_enemy(Piece *piece, int position)
{
    Piece *other_piece = find_piece_by_position(position);

    if (piece->color != other_piece->color) {
        return 1;
    }
    return 0;
}

// Deze functie generiek maken -> zie print_bitboard()
Piece *find_piece_by_position(int position)
{
    if (position < 0 || position > 63) {
        return NULL;
    }
    Piece *pieces = get_pieces();
    uint64_t mask = ((int64_t) 1 << position);
    for (int i = 0; i < 12; i++) {
        if (*(pieces[i].pos_bb) & mask) {
            return &pieces[i];
        }
    }
    return NULL;
}

uint64_t find_possible_pawn_moves(Piece *piece, Square input_square,
                                  int position, uint64_t full_board,
                                  GameState *game_state)
{
    uint64_t possible_moves = (uint64_t) 0;

    if (piece->symbol == 'P') {
        if (input_square.row < 8) {
            if (!(is_bit_set(full_board, (position + 8)))) {
                possible_moves |= ((uint64_t) 1 << (position + 8));
            }
            if (input_square.row == 1) {
                // Double move for first row
                if (!(is_bit_set(full_board, (position + 16))) &&
                    !(is_bit_set(full_board, (position + 8)))) {
                    possible_moves |= ((uint64_t) 1 << (position + 16));
                }
            }
            if (input_square.row == 4) {
                // Implement en passant
                if (game_state->last_moved_piece == 'p' &&
                    abs(input_square.file -
                        (game_state->last_move[2] - FILE_OFFSET)) == 1 &&
                    game_state->last_move[3] == '5') {
                    int new_pos =
                        8 - (input_square.file -
                             (game_state->last_move[2] - FILE_OFFSET));
                    possible_moves |= ((uint64_t) 1 << (position + new_pos));
                    game_state->play_en_passant = 1;
                }
            }
            if (input_square.file < 7 &&
                is_bit_set(full_board, (position + 9)) &&
                is_enemy(piece, (position + 9))) {
                possible_moves |= ((uint64_t) 1 << (position + 9));
            }
            if (input_square.file > 0 &&
                is_bit_set(full_board, (position + 7)) &&
                is_enemy(piece, (position + 7))) {
                possible_moves |= ((uint64_t) 1 << (position + 7));
            }
        }
    } else {
        if (input_square.row > 0) {
            if (!(is_bit_set(full_board, (position - 8)))) {
                possible_moves |= ((uint64_t) 1 << (position - 8));
            }
            if (input_square.row == 6) {
                if (!(is_bit_set(full_board, (position - 16))) &&
                    !(is_bit_set(full_board, (position - 8)))) {
                    possible_moves |= ((uint64_t) 1 << (position - 16));
                }
            }
            if (input_square.row == 3) {
                // Implement en passant
                if (game_state->last_moved_piece == 'P' &&
                    abs(input_square.file -
                        (game_state->last_move[2] - FILE_OFFSET)) == 1 &&
                    game_state->last_move[3] == '4') {
                    int new_pos =
                        8 + (input_square.file -
                             (game_state->last_move[2] - FILE_OFFSET));
                    possible_moves |= ((uint64_t) 1 << (position - new_pos));
                    game_state->play_en_passant = 1;
                }
            }
            if (input_square.file < 7 &&
                is_bit_set(full_board, (position - 7)) &&
                is_enemy(piece, (position - 7))) {
                possible_moves |= ((uint64_t) 1 << (position - 7));
            }
            if (input_square.file > 0 &&
                is_bit_set(full_board, (position - 9)) &&
                is_enemy(piece, (position - 9))) {
                possible_moves |= ((uint64_t) 1 << (position - 9));
            }
        }
    }
    return possible_moves;
}

int check_diag_move(int position, int next_pos)
{
    if (next_pos < 0 || next_pos > 63) {
        return 0;
    }

    int next_row = next_pos / 8;
    int old_row = position / 8;
    int row_diff = next_row - old_row;
    if (row_diff == 1 || row_diff == -1) {
        return 1;
    }
    return 0;
}

void find_diagonal_moves(int position, uint64_t full_board,
                         uint64_t *possible_moves, Piece *piece,
                         int max_counter)
{
    int directions[4] = {7, 9, -7, -9};
    for (int i = 0; i < 4; i++) {
        int counter = 1;
        int next_pos = position + directions[i];
        int old_pos = position;
        while (check_diag_move(old_pos, next_pos) && counter <= max_counter) {
            counter++;
            if (is_bit_set(full_board, next_pos)) {
                if (is_enemy(piece, next_pos)) {
                    *possible_moves |= (uint64_t) 1 << next_pos;
                }
                break;
            }
            *possible_moves |= (uint64_t) 1 << next_pos;
            old_pos = next_pos;
            next_pos = old_pos + directions[i];
        }
    }
}

uint64_t find_possible_bishop_moves(Piece *piece, int position,
                                    uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(position, full_board, &possible_moves, piece,
                        max_counter);
    return possible_moves;
}

int check_vertical_move(int next_pos)
{
    return (next_pos < 0 || next_pos > 63) ? 0 : 1;
}

int check_horizontal_move(int position, int next_pos)
{
    if (next_pos < 0 || next_pos > 63) {
        return 0;
    }
    int next_row = next_pos / 8;
    int old_row = position / 8;
    int row_diff = next_row - old_row;

    if (row_diff == 0) {
        return 1;
    }
    return 0;
}

void find_orthogonal_moves(int position, uint64_t full_board,
                           uint64_t *possible_moves, Piece *piece,
                           int max_counter)
{
    int hor_dir[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        int counter = 1;
        int next_pos = position + hor_dir[i];
        int old_pos = position;
        while (check_horizontal_move(old_pos, next_pos) &&
               counter <= max_counter) {
            counter++;
            if (is_bit_set(full_board, next_pos)) {
                if (is_enemy(piece, next_pos)) {
                    *possible_moves |= (uint64_t) 1 << next_pos;
                }
                break;
            }
            *possible_moves |= (uint64_t) 1 << next_pos;
            old_pos = next_pos;
            next_pos = old_pos + hor_dir[i];
        }
    }

    int ver_dir[2] = {-8, 8};
    for (int i = 0; i < 2; i++) {
        int counter = 1;
        int next_pos = position + ver_dir[i];
        int old_pos = position;
        while (check_vertical_move(next_pos) && counter <= max_counter) {
            counter++;
            if (is_bit_set(full_board, next_pos)) {
                if (is_enemy(piece, next_pos)) {
                    *possible_moves |= (uint64_t) 1 << next_pos;
                }
                break;
            }
            *possible_moves |= (uint64_t) 1 << next_pos;
            old_pos = next_pos;
            next_pos = old_pos + ver_dir[i];
        }
    }
}

uint64_t find_possible_rook_moves(Piece *piece, int position,
                                  uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_orthogonal_moves(position, full_board, &possible_moves, piece,
                          max_counter);
    return possible_moves;
}

uint64_t find_possible_queen_moves(Piece *piece, int position,
                                   uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(position, full_board, &possible_moves, piece,
                        max_counter);
    find_orthogonal_moves(position, full_board, &possible_moves, piece,
                          max_counter);
    return possible_moves;
}

uint64_t find_possible_knight_moves(Piece *piece, int position,
                                    uint64_t full_board)
{
    uint64_t possible_moves = (uint64_t) 0;
    int ver_dir[2] = {-8, 8};
    int hor_step[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        int pos_1 = position + 2 * ver_dir[i];
        if (check_vertical_move(pos_1)) {
            for (int j = 0; j < 2; j++) {
                int new_pos = pos_1 + hor_step[j];
                if (check_horizontal_move(pos_1, new_pos)) {
                    if (is_bit_set(full_board, new_pos) &&
                        !(is_enemy(piece, new_pos))) {
                        continue;
                    }
                    possible_moves |= (uint64_t) 1 << new_pos;
                }
            }
        }
        int pos_2 = position + 2 * hor_step[i];
        if (check_horizontal_move(position, pos_2)) {
            for (int j = 0; j < 2; j++) {
                int new_hor = pos_2 + ver_dir[j];
                if (check_vertical_move(new_hor)) {
                    if (is_bit_set(full_board, new_hor) &&
                        !(is_enemy(piece, new_hor))) {
                        continue;
                    }
                    possible_moves |= (uint64_t) 1 << new_hor;
                }
            }
        }
    }
    return possible_moves;
}

uint64_t find_possible_king_moves(Piece *piece, int position,
                                  uint64_t full_board, GameState *game_state)
{
    
    int max_counter = 1;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(position, full_board, &possible_moves, piece,
                        max_counter);
    find_orthogonal_moves(position, full_board, &possible_moves, piece,
                          max_counter);
    return possible_moves;
}

uint64_t find_possible_moves(Square input_square, Piece *piece,
                             GameState *game_state)
{
    int position = get_position(input_square.file, input_square.row);
    uint64_t full_board = get_full_board();
    uint64_t possible_moves;

    switch (piece->symbol) {
    case 'P':
    case 'p':
        possible_moves = find_possible_pawn_moves(piece, input_square, position,
                                                  full_board, game_state);
        break;
    case 'B':
    case 'b':
        possible_moves =
            find_possible_bishop_moves(piece, position, full_board);
        break;
    case 'R':
    case 'r':
        possible_moves = find_possible_rook_moves(piece, position, full_board);
        break;
    case 'Q':
    case 'q':
        possible_moves = find_possible_queen_moves(piece, position, full_board);
        break;
    case 'N':
    case 'n':
        possible_moves =
            find_possible_knight_moves(piece, position, full_board);
        break;
    case 'K':
    case 'k':
        possible_moves =
            find_possible_king_moves(piece, position, full_board, game_state);
        break;
    default:
        possible_moves = (uint64_t) 0;
        break;
    }

    return possible_moves;
}

Piece *get_piece_bb(char piece)
{
    Piece *pieces = get_pieces();
    for (int i = 0; i < 12; i++) {
        if (pieces[i].symbol == piece) {
            return &pieces[i];
        }
    }
    return NULL;
}