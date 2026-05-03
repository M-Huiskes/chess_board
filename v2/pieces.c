#include "pieces.h"

#include "bit_utils.h"
#include "board.h"
#include "state.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

static void print_bitboard(uint64_t bitboard)
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

int is_enemy(char piece_color, int position, BoardState *board)
{
    Piece *other_piece = get_piece_by_position(position, board);

    if (piece_color != other_piece->color) {
        return 1;
    }
    return 0;
}

int get_direction_pawn_move(int position, char color, int increment)
{
    return color == 'w' ? position + increment : position - increment;
}

uint64_t find_possible_pawn_moves(GameState *game_state, uint64_t full_board)
{
    uint64_t possible_moves = (uint64_t) 0;
    char color_playing = game_state->selected_piece->color;
    int position = game_state->bit_position;
    MoveHistory move_history = game_state->move_history;

    int is_first_move;
    if (color_playing == 'w') {
        is_first_move = game_state->selected_square.row == 1 ? 1 : 0;
    } else {
        is_first_move = game_state->selected_square.row == 6 ? 1 : 0;
    }

    // Check one move forward
    int direction_one = get_direction_pawn_move(position, color_playing, 8);
    if (!(is_bit_set(full_board, direction_one))) {
        possible_moves |= ((uint64_t) 1 << direction_one);

        int direction_two =
            get_direction_pawn_move(position, color_playing, 16);
        // Check double move forward when pawn is still on home square
        if (is_first_move && !(is_bit_set(full_board, direction_two))) {
            possible_moves |= ((uint64_t) 1 << direction_two);
        }
    }

    // Generate possible taking moves
    int take_directions[2] = {
        get_direction_pawn_move(position, color_playing, 7),
        get_direction_pawn_move(position, color_playing, 9)};

    for (int i = 0; i < 2; i++) {

        if (take_directions[i] > 63 || take_directions[i] < 0 ||
            abs(take_directions[i] / 8 - position / 8) != 1) {
            continue;
        }
        if (is_bit_set(full_board, take_directions[i]) &&
            is_enemy(color_playing, take_directions[i], &(game_state->board))) {
            possible_moves |= ((uint64_t) 1 << take_directions[i]);
        }
    }

    // Generate en passant moves
    if ((game_state->last_moved_piece == 'p' &&
         game_state->selected_square.row == 4) ||
        (game_state->last_moved_piece == 'P' &&
         game_state->selected_square.row == 3)) {
        uint16_t last_move = move_history.moves[move_history.count - 1];
        int to_position = get_to(last_move);
        int from_position = get_from(last_move);

        if (abs(position - to_position) == 1 &&
            abs(to_position - from_position) == 16) {
            possible_moves |= (uint64_t) 1 << get_direction_pawn_move(
                                  to_position, color_playing, 8);
            game_state->en_passant_possible = 1;
        }
    }

    return possible_moves;
}

uint64_t find_possible_moves(GameState *game_state)
{
    uint64_t full_board = get_full_bit_board(&(game_state->board));
    uint64_t possible_moves;

    switch (game_state->selected_piece->symbol) {
    case 'P':
    case 'p':
        possible_moves = find_possible_pawn_moves(game_state, full_board);
        break;

    default:
        possible_moves = (uint64_t) 0;
        break;
    }
    return possible_moves;
}