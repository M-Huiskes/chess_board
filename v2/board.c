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
    team_state->pin_info = malloc(8 * sizeof(PinnedInfo));
    team_state->count_pinned_pieces = 0;
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

void print_bitboard(uint64_t bitboard)
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

void determine_check_info(GameState *game_state, uint64_t possible_moves)
{
    char color_playing = game_state->selected_piece->color;
    Piece enemy_king;

    if (color_playing == 'w') {
        enemy_king = game_state->board.black.pieces[KING_ARRAY_INDEX];
    } else {
        enemy_king = game_state->board.white.pieces[KING_ARRAY_INDEX];
    }

    int enemy_king_position = get_lowest_bit_index(enemy_king.pos_bb);

    while (possible_moves) {
        int position = get_lowest_bit_index(possible_moves);
        if (position == enemy_king_position) {
            if (!(game_state->check_info.is_check)) {
                CheckInfo check_info = {
                    .is_check = 1,
                    .check_by = game_state->selected_piece->symbol,
                    .is_double_check = 0,
                    .position_check = game_state->bit_position,
                };
                game_state->check_info = check_info;
            } else {
                game_state->check_info.is_double_check = 1;
            }
        }
        possible_moves &= possible_moves - 1;
    }
}

uint64_t calculate_attack_map(GameState *game_state, char color_playing,
                              int run_check_info)
{
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

            // Check whether enemy king is attacked in possible moves
            if (run_check_info) {
                determine_check_info(game_state, possible_moves);
            }

            attack_map |= possible_moves;
            piece_bb &= piece_bb - 1;
        }
    }
    return attack_map;
}

int validate_direction_valid(int direction, int old_pos, int new_pos)
{
    if (abs(direction) == 1) {
        return check_horizontal_move(old_pos, new_pos);
    }
    if (abs(direction) == 7 || abs(direction) == 9) {
        return check_diag_move(old_pos, new_pos);
    }

    if (abs(direction) == 8) {
        return check_vertical_move(new_pos);
    }

    return 0;
}

void calculate_pinned_pieces(GameState *game_state)
{
    char color_playing = game_state->selected_piece->color;
    TeamState *team_state = color_playing == 'w' ? &(game_state->board.black)
                                                 : &(game_state->board.white);
    Piece enemy_king = team_state->pieces[KING_ARRAY_INDEX];
    int king_position = get_lowest_bit_index(enemy_king.pos_bb);
    uint64_t full_board = get_full_bit_board(&(game_state->board));
    int count_pinned_pieces = 0;
    // TODO: Check if I can reuse find_diagonal_moves for this, shares the
    // same functionality
    int directions[8] = {7, 9, -7, -9, -1, 1, 8, -8};
    for (int i = 0; i < 8; i++) {
        int next_pos = king_position + directions[i];
        int old_pos = king_position;
        int one_friendly_piece = 0;
        int pinned_piece_pos;

        while (validate_direction_valid(directions[i], old_pos, next_pos)) {
            if (is_bit_set(full_board, next_pos)) {
                if (!(is_enemy(enemy_king.color, next_pos,
                               &(game_state->board)))) {
                    if (!(one_friendly_piece)) {
                        one_friendly_piece = 1;
                        pinned_piece_pos = next_pos;
                    } else {
                        break;
                    }
                }
                if (is_enemy(enemy_king.color, next_pos,
                             &(game_state->board))) {
                    Piece *piece =
                        get_piece_by_position(next_pos, &(game_state->board));

                    if (piece == NULL) {
                        old_pos = next_pos;
                        next_pos = old_pos + directions[i];
                        continue;
                    }
                    if ((tolower(piece->symbol) == 'q' ||
                         (tolower(piece->symbol) == 'b' &&
                          (abs(directions[i]) == 7 ||
                           abs(directions[i]) == 9)) ||
                         (tolower(piece->symbol) == 'r' &&
                          (abs(directions[i]) == 1 ||
                           abs(directions[i]) == 8))) &&
                        one_friendly_piece) {
                        PinnedInfo pin_info = {
                            .pinned_position = pinned_piece_pos,
                            .pinner_position = next_pos,
                            .direction = directions[i],
                        };
                        team_state->pin_info[count_pinned_pieces] = pin_info;
                        count_pinned_pieces++;
                        break;
                    } else {
                        break;
                    }
                }
            }
            old_pos = next_pos;
            next_pos = old_pos + directions[i];
        }
    }
    team_state->count_pinned_pieces = count_pinned_pieces;
}

void validate_check_fixed(GameState *game_state)
{
    char color_playing = game_state->selected_piece->color;

    Piece king;
    uint64_t attack_map;
    int run_check_info = 0;

    if (color_playing == 'w') {
        king = game_state->board.white.pieces[KING_ARRAY_INDEX];
        attack_map = calculate_attack_map(game_state, 'b', run_check_info);
    } else {
        king = game_state->board.black.pieces[KING_ARRAY_INDEX];
        attack_map = calculate_attack_map(game_state, 'w', run_check_info);
    }

    if (king.pos_bb & attack_map) {
        printf("Check not fixed after move");
        exit(EXIT_FAILURE);
    }

    game_state->check_info = (CheckInfo){
        .check_by = '0',
        .is_check = 0,
        .is_double_check = 0,
        .position_check = -1,
    };
}

void update_castling_state(GameState *game_state, char color)
{
    TeamState *team_state = get_team_state_by_color(game_state, color);
    char selected_piece = tolower(game_state->selected_piece->symbol);

    if (!(team_state->long_castle_allowed) &&
        !(team_state->short_castle_allowed)) {
        return;
    }

    if (selected_piece != 'k' && selected_piece != 'r') {
        return;
    }

    if (selected_piece == 'k') {
        team_state->short_castle_allowed = 0;
        team_state->long_castle_allowed = 0;
        return;
    }

    int selected_pos = game_state->bit_position;
    if (color == 'b') {
        if (selected_pos == BLACK_LEFT_ROOK_START) {
            team_state->long_castle_allowed = 0;
        }
        if (selected_pos == BLACK_RIGHT_ROOK_START) {
            team_state->short_castle_allowed = 0;
        }
    } else {
        if (selected_pos == WHITE_LEFT_ROOK_START) {
            team_state->long_castle_allowed = 0;
        }
        if (selected_pos == WHITE_RIGHT_ROOK_START) {
            team_state->short_castle_allowed = 0;
        }
    }
}

int is_castling_move(GameState *game_state)
{
    if (tolower(game_state->selected_piece->symbol) != 'k') {
        return 0;
    }

    char color_playing = game_state->selected_piece->color;
    int king_position =
        color_playing == 'w' ? WHITE_KING_START : BLACK_KING_START;
    TeamState *team_state = get_team_state_by_color(game_state, color_playing);

    if (game_state->output_position - king_position == -2 &&
        team_state->long_castle_allowed) {

        team_state->long_castle_allowed = 0;
        team_state->short_castle_allowed = 0;

        return CASTLE_QUEEN;
    }

    if (game_state->output_position - king_position == 2 &&
        team_state->short_castle_allowed) {

        team_state->long_castle_allowed = 0;
        team_state->short_castle_allowed = 0;

        return CASTLE_KING;
    }

    return 0;
}

void make_move(GameState *game_state)
{
    int is_check_initially = game_state->check_info.is_check;
    int output_position = game_state->output_position;
    int input_position = game_state->bit_position;
    int move_type = QUIET;
    char captured_piece_symbol = '0';
    char color_playing = game_state->selected_piece->color;
    int is_castling = is_castling_move(game_state);

    if (is_castling) {
        move_type = is_castling;
        // Move the king
        unset_bit(&(game_state->selected_piece->pos_bb), input_position);
        set_bit(&(game_state->selected_piece->pos_bb), output_position);

        Piece *rook;
        int rook_position;
        int new_rook_position;

        if (move_type == CASTLE_KING) {
            rook_position = color_playing == 'w' ? WHITE_RIGHT_ROOK_START
                                                 : BLACK_RIGHT_ROOK_START;
            rook = get_piece_by_position(rook_position, &(game_state->board));
            new_rook_position = output_position - 1;
        }
        if (move_type == CASTLE_QUEEN) {
            rook_position = color_playing == 'w' ? WHITE_LEFT_ROOK_START
                                                 : BLACK_LEFT_ROOK_START;
            rook = get_piece_by_position(rook_position, &(game_state->board));
            new_rook_position = output_position + 1;
        }

        // Move the rook
        unset_bit(&(rook->pos_bb), rook_position);
        set_bit(&(rook->pos_bb), new_rook_position);

    } else {

        update_castling_state(game_state, color_playing);

        Piece *other_piece =
            get_piece_by_position(output_position, &(game_state->board));
        if (other_piece != NULL) {
            unset_bit(&(other_piece->pos_bb), output_position);
            move_type = CAPTURE;
            captured_piece_symbol = other_piece->symbol;
        }

        if (game_state->en_passant_possible && other_piece == NULL) {
            move_type = handle_en_passant(game_state, output_position);
            captured_piece_symbol =
                color_to_move(game_state) == 'w' ? 'p' : 'P';
        }

        unset_bit(&(game_state->selected_piece->pos_bb), input_position);
        if (game_state->promote_to != '0') {
            move_type =
                handle_promotion_move(game_state, output_position, move_type);
        } else {
            set_bit(&(game_state->selected_piece->pos_bb), output_position);
        }
    }

    push_move(&(game_state->move_history),
              encode_move(input_position, output_position, move_type,
                          captured_piece_symbol));
    game_state->last_moved_piece = game_state->selected_piece->symbol;

    // Attack map updates the selected piece, but it is always the same color as
    // for which team we are making a move
    // This also determines check info!
    uint64_t attack_map = calculate_attack_map(game_state, color_playing, 1);

    if (color_playing == 'w') {
        game_state->board.white.attack_map = attack_map;
    } else {
        game_state->board.black.attack_map = attack_map;
    }

    // Pinned pieces does not change game state
    // Pinned pieces can only move along the direction from which they are
    // pinned
    calculate_pinned_pieces(game_state);

    // Is double check? If so, check can only be countered by moving king

    // Check by pawn / knight -> only stoppable by
    // capturing said pawn/knight (or moving king)

    // Validate whether check has been fixed after making move and reset check
    // info
    if (is_check_initially) {
        validate_check_fixed(game_state);
        if (game_state->check_info.is_check) {
            exit(EXIT_FAILURE);
        }
    }

    // TODO: Nog fixen dat als koning schaak staat door diagonaal / horizontaal
    // Dat dan squares achter attack ook niet beschikbaar zijn
}

int has_game_ended(GameState *game_state)
{
    char color_playing = game_state->move_history.count % 2 == 0 ? 'w' : 'b';
    int attack_moves_only = 0;
    printf("Reached game ended with %c color playing\n", color_playing);

    for (int i = 0; i < 12; i++) {
        if (color_playing == 'w' && i >= 6) {
            continue;
        }
        if (color_playing == 'b' && i < 6) {
            continue;
        }

        Piece *piece = get_piece_by_index(i, &(game_state->board));
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

            if (game_state->check_info.is_check) {
                possible_moves =
                    validate_moves_in_check(game_state, possible_moves);
            }

            if (possible_moves) {
                printf("Found possible moves for %c\n",
                       game_state->selected_piece->symbol);
                print_bitboard(possible_moves);
                return PLAYING;
            }
            piece_bb &= piece_bb - 1;
        }
    }

    if (game_state->check_info.is_check) {
        return CHECK_MATE; 
    } else {
        return STALE_MATE;
    }
}