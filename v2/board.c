#include "board.h"

#include "bit_utils.h"
#include "pieces.h"
#include "state.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char color_to_move(GameState *game_state)
{
    return game_state->move_history.count % 2 == 0 ? 'w' : 'b';
}

Piece *get_piece_by_index(int index, BoardState *board)
{
    if (index < 6) {
        return &(board->white.pieces[index]);
    } else {
        return &(board->black.pieces[index - 6]);
    }
}

int position_from_square(Square *input_square)
{
    return input_square->row * 8 + input_square->file;
}

Square square_from_position(int position)
{
    int row = position / 8;
    int file = position % 8;

    return (Square){file, row};
}

uint64_t get_full_bit_board(BoardState *board)
{
    uint64_t full_board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, board);
        full_board |= piece->pos_bb;
    }
    return full_board;
}

uint64_t get_full_team_bit_board(BoardState *board, char color)
{
    uint64_t full_board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
        if (color == 'w' && i >= 6) {
            continue;
        }
        if (color == 'b' && i < 6) {
            continue;
        }
        Piece *piece = get_piece_by_index(i, board);
        full_board |= piece->pos_bb;
    }
    return full_board;
}

Piece *get_piece_by_square(Square *input_square, BoardState *board)
{
    int position = position_from_square(input_square);

    return get_piece_by_position(position, board);
}

Piece *get_piece_by_position(int position, BoardState *board)
{
    if (position < 0 || position > 63) {
        return NULL;
    }

    uint64_t mask = (uint64_t) 1 << position;

    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, board);
        if (piece->pos_bb & mask) {
            return piece;
        }
    }

    return NULL;
}

Piece *get_piece_by_symbol(char symbol, BoardState *board)
{

    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, board);
        if (piece->symbol == symbol) {
            return piece;
        }
    }
    return NULL;
}

int is_enemy(char piece_color, int position, BoardState *board)
{
    Piece *other_piece = get_piece_by_position(position, board);

    if (piece_color != other_piece->color) {
        return 1;
    }
    return 0;
}

void init_team_state(TeamState *team_state, char color)
{
    team_state->short_castle_allowed = 1;
    team_state->long_castle_allowed = 1;
    team_state->attack_map = (uint64_t) 0;
    // team_state->pin_info = malloc(8 * sizeof(PinnedInfo));
    init_pieces(team_state->pieces, color);
}

BoardState init_board(void)
{
    BoardState board;
    init_team_state(&(board.white), 'w');
    init_team_state(&(board.black), 'b');

    return board;
}

int handle_en_passant(GameState *game_state, int output_position)
{
    int input_position = game_state->bit_position;
    int played_en_passant = QUIET;
    if (abs(input_position - output_position) != 8) {

        int other_pawn_pos = game_state->selected_piece->color == 'w'
                                 ? output_position - 8
                                 : output_position + 8;
        Piece *other_pawn =
            get_piece_by_position(other_pawn_pos, &(game_state->board));

        if (other_pawn != NULL) {
            char other_pawn_symbol =
                game_state->selected_piece->color == 'w' ? 'p' : 'P';
            if (other_pawn->symbol != other_pawn_symbol) {
                fprintf(stderr, "Error: en passant target is not a pawn\n");
                exit(EXIT_FAILURE);
            }
            unset_bit(&(other_pawn->pos_bb), other_pawn_pos);
            played_en_passant = EN_PASSANT;
        } else {
            fprintf(stderr, "Did not find a piece to capture in en passant\n");
            exit(EXIT_FAILURE);
        }
    }
    game_state->en_passant_possible = 0;
    return played_en_passant;
}

int handle_promotion_move(GameState *game_state, int output_position,
                          int current_move_type)
{
    Piece *promotion_piece =
        get_piece_by_symbol(game_state->promote_to, &(game_state->board));

    set_bit(&(promotion_piece->pos_bb), output_position);
    int move_type;

    switch (game_state->promote_to) {
    case 'N':
    case 'n':
        if (current_move_type == CAPTURE) {
            move_type = PROMO_CAPTURE_KNIGHT;
        } else {
            move_type = PROMO_KNIGHT;
        }
        break;
    case 'R':
    case 'r':
        if (current_move_type == CAPTURE) {
            move_type = PROMO_CAPTURE_ROOK;
        } else {
            move_type = PROMO_ROOK;
        }
        break;
    case 'Q':
    case 'q':
        if (current_move_type == CAPTURE) {
            move_type = PROMO_CAPTURE_QUEEN;
        } else {
            move_type = PROMO_QUEEN;
        }
        break;
    case 'B':
    case 'b':
        if (current_move_type == CAPTURE) {
            move_type = PROMO_CAPTURE_BISHOP;
        } else {
            move_type = PROMO_BISHOP;
        }
        break;
    default:
        if (current_move_type == CAPTURE) {
            move_type = PROMO_CAPTURE_QUEEN;
        } else {
            move_type = PROMO_QUEEN;
        }
        break;
    }

    game_state->promote_to = '0';
    return move_type;
}

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

void calculate_attack_map(GameState *game_state)
{
    char color_playing = game_state->selected_piece->color;
    int attack_moves_only = 1;

    uint64_t attack_map = (uint64_t) 0;
    // TODO: Make this faster by incrementally updating attack map (based on
    // last moved pieces etc)
    for (int i = 0; i < 12; i++) {
        if (color_playing == 'w' && i >= 6) {
            continue;
        }
        if (color_playing == 'b' && i < 6) {
            continue;
        }
        Piece *piece = get_piece_by_index(i, &(game_state->board));
        // Loop over each piece and each position where this piece is
        uint64_t piece_bb = piece->pos_bb;
        game_state->selected_piece = piece;
        while (piece_bb) {
            int position = get_lowest_bit_index(piece_bb);
            // Update game_state accordingly, this is ugly!
            // but after make_move all these values are set to default values
            game_state->bit_position = position;
            game_state->selected_square = square_from_position(position);
            uint64_t possible_moves =
                find_possible_moves(game_state, attack_moves_only);
            // TODO: For pawns this is not correct, only check the sqaures they
            // attack!
            attack_map |= possible_moves;
            piece_bb &= piece_bb - 1;
        }
    }

    if (color_playing == 'w') {
        game_state->board.white.attack_map = attack_map;
    } else {
        game_state->board.black.attack_map = attack_map;
    }
}

void is_check(GameState *game_state)
{
    char color_playing = game_state->selected_piece->color;
    Piece enemy_king;
    uint64_t attack_map;

    if (color_playing == 'w') {
        enemy_king = game_state->board.black.pieces[KING_ARRAY_INDEX];
        attack_map = game_state->board.white.attack_map;
    } else {
        enemy_king = game_state->board.white.pieces[KING_ARRAY_INDEX];
        attack_map = game_state->board.black.attack_map;
    }

    if (enemy_king.pos_bb & attack_map) {
        game_state->is_check = 1;
    } else {
        game_state->is_check = 0;
    }
}

void calculate_pinned_pieces(GameState *game_state)
{
    char color_playing = game_state->selected_piece->color;
    TeamState team_state = color_playing == 'w' ? game_state->board.black
                                                : game_state->board.white;
    Piece enemy_king = team_state.pieces[KING_ARRAY_INDEX];
    int king_position = get_lowest_bit_index(enemy_king.pos_bb);
    uint64_t full_board = get_full_bit_board(&(game_state->board));
    int count_pinned_pieces = 0;
    printf("reached here!\n§");
    // TODO: Check if I can reuse find_diagonal_moves for this, shares the
    // same functionality
    int diagonal_moves[4] = {7, 9, -7, -9};
    for (int i = 0; i < 4; i++) {
        int next_pos = king_position + diagonal_moves[i];
        int old_pos = king_position;
        int one_friendly_piece = 0;
        printf("Loop number %d\n", i);
        while (check_diag_move(old_pos, next_pos)) {
            if (is_bit_set(full_board, next_pos)) {
                if (!(is_enemy(enemy_king.color, next_pos,
                               &(game_state->board)))) {
                    if (!(one_friendly_piece)) {
                        one_friendly_piece = 1;
                    } else {
                        one_friendly_piece = 0;
                    }
                }
                if (is_enemy(enemy_king.color, next_pos,
                             &(game_state->board))) {
                    Piece *piece =
                        get_piece_by_position(next_pos, &(game_state->board));

                    if (piece == NULL) {
                        continue;
                    }
                    if ((tolower(piece->symbol) == 'q' ||
                         tolower(piece->symbol) == 'b') &&
                        one_friendly_piece) {
                        PinnedInfo pin_info = {
                            .bit_position = next_pos,
                            .direction = diagonal_moves[i],
                        };
                        // team_state.pin_info[count_pinned_pieces] = pin_info;
                        count_pinned_pieces++;
                        printf("Found pinned piece!!!\n");
                    }
                }
            }
            old_pos = next_pos;
            next_pos = old_pos + diagonal_moves[i];
        }
    }
    printf("Number of pinned pieces %d\n", count_pinned_pieces);

    // TODO: Check if I can reuse find_horizontal_moves for this, shares the
    // same functionality
    // int directions[4] = {-1, 8, 1, -8};
    // for (int i = 0; i < 4; i++) {
    // }
}

void make_move(GameState *game_state)
{
    int output_position = game_state->output_position;
    int input_position = game_state->bit_position;
    int move_type = QUIET;
    char captured_piece_symbol = '0';

    Piece *other_piece =
        get_piece_by_position(output_position, &(game_state->board));
    if (other_piece != NULL) {
        unset_bit(&(other_piece->pos_bb), output_position);
        move_type = CAPTURE;
        captured_piece_symbol = other_piece->symbol;
    }

    if (game_state->en_passant_possible && other_piece == NULL) {
        move_type = handle_en_passant(game_state, output_position);
        captured_piece_symbol = color_to_move(game_state) == 'w' ? 'p' : 'P';
    }

    unset_bit(&(game_state->selected_piece->pos_bb), input_position);
    if (game_state->promote_to != '0') {
        move_type =
            handle_promotion_move(game_state, output_position, move_type);
    } else {
        set_bit(&(game_state->selected_piece->pos_bb), output_position);
    }

    push_move(&(game_state->move_history),
              encode_move(input_position, output_position, move_type,
                          captured_piece_symbol));
    game_state->last_moved_piece = game_state->selected_piece->symbol;

    calculate_attack_map(game_state);
    calculate_pinned_pieces(game_state);
    is_check(game_state);

    // Calculate pinned pieces for other color by position -> these can't move
    // next move

    // Is double check? Checken Check diagonal/horizontal line from
    // which check is generated. If so, check can either be countered by moving
    // on this line (or moving king)

    // Check by pawn / knight -> only stoppable by
    // capturing said pawn/knight (or moving king)
}