#include "board.h"

#include "bit_utils.h"
#include "pieces.h"
#include "state.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

uint64_t get_full_bit_board(BoardState *board)
{
    uint64_t full_board = (uint64_t) 0;
    for (int i = 0; i < 12; i++) {
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

void init_team_state(TeamState *team_state, char color)
{
    team_state->short_castle_allowed = 1;
    team_state->long_castle_allowed = 1;
    team_state->attack_map = (uint64_t) 0;
    init_pieces(team_state->pieces, color);
}

BoardState init_board(void)
{
    BoardState board;
    init_team_state(&(board.white), 'w');
    init_team_state(&(board.black), 'b');

    return board;
}

void make_move(GameState *game_state, int output_position)
{
    int input_position = game_state->bit_position;
    int move_type = QUIET;

    Piece *other_piece =
        get_piece_by_position(output_position, &(game_state->board));
    if (other_piece != NULL) {
        unset_bit(&(other_piece->pos_bb), output_position);
        move_type = CAPTURE;
    }

    if (game_state->en_passant_possible) {
        if (abs(input_position - output_position) != 8 && other_piece == NULL) {

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
                unset_bit(&(other_pawn->pos_bb),
                          other_pawn_pos);
            } else {
                fprintf(stderr,
                        "Did not find a piece to capture in en passant\n");
                exit(EXIT_FAILURE);
            }
        }
        game_state->en_passant_possible = 0;
    }

    printf("Selected piece symbol %c\n", game_state->selected_piece->symbol);

    unset_bit(&(game_state->selected_piece->pos_bb), input_position);
    set_bit(&(game_state->selected_piece->pos_bb), output_position);

    push_move(&(game_state->move_history),
              encode_move(input_position, output_position, move_type));
    game_state->last_moved_piece = game_state->selected_piece->symbol;

    uint16_t last_move =
        game_state->move_history.moves[game_state->move_history.count - 1];

    printf("Last move: from %d, to %d, flags %d\n", get_from(last_move),
           get_to(last_move), get_flags(last_move));
    printf("Number of moves: %d\n", game_state->move_history.count);
}