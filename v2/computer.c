#include "computer.h"

#include "bit_utils.h"
#include "board.h"
#include "evaluation.h"
#include "state.h"

#include <stdio.h>

int minimax(GameState *game_state, int depth, int is_maxing);

static int eval_moves(GameState *game_state, int piece_start, int piece_end,
                      int depth, int is_maxing, int best_score)
{
    printf("Running eval_moves for depth %d and is_maxing %d\n", depth,
           is_maxing);
    for (int i = piece_start; i < piece_end; i++) {
        printf("Running for index %d\n", i);
        Piece *piece = get_piece_by_index(i, &(game_state->board));
        uint64_t piece_bb = piece->pos_bb;
        game_state->selected_piece = piece;

        while (piece_bb) {
            // Loop over all pieces and use their position as input square
            int position = get_lowest_bit_index(piece_bb);
            game_state->selected_square = square_from_position(position);
            game_state->bit_position = position;

            // Find possible moves for piece on input square
            uint64_t possible_moves = find_possible_moves(game_state, 0);
            if (game_state->check_info.is_check) {
                possible_moves =
                    validate_moves_in_check(game_state, possible_moves);
            }

            // Copy parts of the game state to recopy after move generation
            OldState old_state = write_old_state(game_state);

            // Loop over possible moves, make the move and continue recursively
            Square output_square = (Square){-1, -1};
            while (possible_moves) {
                output_square =
                    square_from_position(get_lowest_bit_index(possible_moves));

                game_state->output_position =
                    get_lowest_bit_index(possible_moves);
                make_move(game_state);
                // TODO: Fix this, this is very ugly, just to check whether it
                // now works correctly!!!
                game_state->selected_square = square_from_position(position);
                game_state->bit_position = position;
                game_state->selected_piece = piece;

                int score = minimax(game_state, depth - 1, !is_maxing);

                if (is_maxing ? score > best_score : score < best_score) {
                    best_score = score;
                    game_state->computer_move = (ComputerMove){
                        .from = game_state->bit_position,
                        .to = game_state->output_position,
                        .piece = game_state->selected_piece->symbol,
                    };
                    printf(
                        "Computer move, piece %c from %d to %d with score %d\n",
                        game_state->computer_move.piece,
                        game_state->computer_move.from,
                        game_state->computer_move.to, score);
                }
                unmake_move(game_state, old_state);
                possible_moves &= possible_moves - 1;

            }
            piece_bb &= piece_bb - 1;

        }
    }
    return best_score;
}

int minimax(GameState *game_state, int depth, int is_maxing)
{
    printf("Running minimax for depth %d\n", depth);
    if (depth == 0) {
        // printf("Evaluation at depth 0 %d\n",
        // calculate_evaluation(game_state));
        return calculate_evaluation(game_state);
    }
    // Always end on move of opponent
    // depth = 2 * depth;

    int best_score = is_maxing ? -9999 : 9999;
    int piece_start = is_maxing ? 0 : 6;
    int piece_end = is_maxing ? 6 : 12;

    return eval_moves(game_state, piece_start, piece_end, depth, is_maxing,
                      best_score);
}