#include "board.h"
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

    int game_end = has_game_ended(game_state);
    if (game_end == CHECK_MATE) {
        char color_playing =
            game_state->move_history.count % 2 == 0 ? 'w' : 'b';
        int max_score = color_playing == 'w' ? 9999 : -9999;
        evaluation = max_score;
    }

    if (game_end == STALE_MATE) {
        evaluation = 0;
    }

    return evaluation;
}