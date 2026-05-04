#include "pieces.h"

#include "bit_utils.h"
#include "board.h"
#include "state.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const uint64_t START_WHITE_PAWNS = 0x000000000000FF00ULL;
const uint64_t START_BLACK_PAWNS = 0x00FF000000000000ULL;
const uint64_t START_WHITE_ROOKS = 0x0000000000000081ULL;
const uint64_t START_BLACK_ROOKS = 0x8100000000000000ULL;
const uint64_t START_WHITE_KNIGHTS = 0x0000000000000042ULL;
const uint64_t START_BLACK_KNIGHTS = 0x4200000000000000ULL;
const uint64_t START_WHITE_BISHOPS = 0x0000000000000024ULL;
const uint64_t START_BLACK_BISHOPS = 0x2400000000000000ULL;
const uint64_t START_WHITE_QUEEN = 0x0000000000000010ULL;
const uint64_t START_BLACK_QUEEN = 0x1000000000000000ULL;
const uint64_t START_WHITE_KING = 0x0000000000000008ULL;
const uint64_t START_BLACK_KING = 0x0800000000000000ULL;

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

TeamState get_team_state_by_color(GameState *game_state, char color)
{
    return color == 'w' ? game_state->board.white : game_state->board.black;
}

TeamState get_enemy_team_state(GameState *game_state, char color)
{
    return color == 'w' ? game_state->board.black : game_state->board.white;
}

int get_direction_pawn_move(int position, char color, int increment)
{
    return color == 'w' ? position + increment : position - increment;
}

uint64_t find_possible_pawn_moves(GameState *game_state, uint64_t full_board,
                                  int attack_moves_only)
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

    if (!(attack_moves_only)) {
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

        // Generate en passant moves
        if ((game_state->last_moved_piece == 'p' &&
             game_state->selected_square.row == 4) ||
            (game_state->last_moved_piece == 'P' &&
             game_state->selected_square.row == 3)) {
            uint16_t last_move =
                move_history.moves[move_history.count - 1].move;
            int to_position = get_to(last_move);
            int from_position = get_from(last_move);

            if (abs(position - to_position) == 1 &&
                abs(to_position - from_position) == 16) {
                possible_moves |= (uint64_t) 1 << get_direction_pawn_move(
                                      to_position, color_playing, 8);
                game_state->en_passant_possible = 1;
            }
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
        if ((is_bit_set(full_board, take_directions[i]) &&
             is_enemy(color_playing, take_directions[i],
                      &(game_state->board))) ||
            attack_moves_only) {
            possible_moves |= ((uint64_t) 1 << take_directions[i]);
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

void find_diagonal_moves(GameState *game_state, int max_counter,
                         uint64_t full_board, uint64_t *possible_moves)
{
    int directions[4] = {7, 9, -7, -9};
    int position = game_state->bit_position;

    for (int i = 0; i < 4; i++) {
        int counter = 1;
        int next_pos = position + directions[i];
        int old_pos = position;
        while (check_diag_move(old_pos, next_pos) && counter <= max_counter) {
            counter++;
            if (is_bit_set(full_board, next_pos)) {
                if (is_enemy(game_state->selected_piece->color, next_pos,
                             &(game_state->board))) {
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

uint64_t find_possible_bishop_moves(GameState *game_state, uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(game_state, max_counter, full_board, &possible_moves);
    return possible_moves;
}

void find_orthogonal_moves(GameState *game_state, int max_counter,
                           uint64_t full_board, uint64_t *possible_moves)
{
    int position = game_state->bit_position;

    int hor_dir[2] = {-1, 1};
    for (int i = 0; i < 2; i++) {
        int counter = 1;
        int next_pos = position + hor_dir[i];
        int old_pos = position;
        while (check_horizontal_move(old_pos, next_pos) &&
               counter <= max_counter) {
            counter++;
            if (is_bit_set(full_board, next_pos)) {
                if (is_enemy(game_state->selected_piece->color, next_pos,
                             &(game_state->board))) {
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
                if (is_enemy(game_state->selected_piece->color, next_pos,
                             &(game_state->board))) {
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

uint64_t find_possible_rook_moves(GameState *game_state, uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_orthogonal_moves(game_state, max_counter, full_board, &possible_moves);
    return possible_moves;
}

uint64_t find_possible_queen_moves(GameState *game_state, uint64_t full_board)
{
    int max_counter = 8;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(game_state, max_counter, full_board, &possible_moves);
    find_orthogonal_moves(game_state, max_counter, full_board, &possible_moves);
    return possible_moves;
}

uint64_t find_possible_knight_moves(GameState *game_state, uint64_t full_board)
{
    int position = game_state->bit_position;
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
                        !(is_enemy(game_state->selected_piece->color, new_pos,
                                   &(game_state->board)))) {
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
                        !(is_enemy(game_state->selected_piece->color, new_hor,
                                   &(game_state->board)))) {
                        continue;
                    }
                    possible_moves |= (uint64_t) 1 << new_hor;
                }
            }
        }
    }
    return possible_moves;
}

uint64_t find_possible_king_moves(GameState *game_state, uint64_t full_board)
{
    Piece *piece = game_state->selected_piece;

    char color_moving = piece->color;

    int max_counter = 1;
    uint64_t possible_moves = (uint64_t) 0;
    find_diagonal_moves(game_state, max_counter, full_board, &possible_moves);
    find_orthogonal_moves(game_state, max_counter, full_board, &possible_moves);

    // Prevent moving in check
    TeamState enemy_team = get_enemy_team_state(game_state, color_moving);
    uint64_t enemy_attack_map = enemy_team.attack_map;

    possible_moves = possible_moves & ~enemy_attack_map;

    // Castling
    return possible_moves;
}

uint64_t find_possible_moves(GameState *game_state, int attack_moves_only)
{
    uint64_t full_board = get_full_bit_board(&(game_state->board));
    uint64_t possible_moves;

    switch (game_state->selected_piece->symbol) {
    case 'P':
    case 'p':
        possible_moves =
            find_possible_pawn_moves(game_state, full_board, attack_moves_only);
        break;
    case 'B':
    case 'b':
        possible_moves = find_possible_bishop_moves(game_state, full_board);
        break;
    case 'R':
    case 'r':
        possible_moves = find_possible_rook_moves(game_state, full_board);
        break;
    case 'Q':
    case 'q':
        possible_moves = find_possible_queen_moves(game_state, full_board);
        break;
    case 'N':
    case 'n':
        possible_moves = find_possible_knight_moves(game_state, full_board);
        break;
    case 'K':
    case 'k':
        possible_moves = find_possible_king_moves(game_state, full_board);
        break;

    default:
        possible_moves = (uint64_t) 0;
        break;
    }

    return possible_moves;
}