#include "state.h"

int calculate_evaluation(GameState *game_state)
{
    int evaluation = 0;
    int piece_value = 0;

    for (int i = 0; i < 12; i++) {
        Piece *piece = get_piece_by_index(i, &(game_state->board));
        uint64_t piece_bb = piece->pos_bb;

        if (piece->color == 'w') {
            piece_value = piece->value;
        } else {
            piece_value = -(piece->value);
        }

        while (piece_bb) {
            piece_bb &= piece_bb - 1;
            evaluation = evaluation + piece_value;
        }
    }

    return evaluation;
}