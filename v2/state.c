#include "state.h"

#include "board.h"

#include <stdio.h>

GameState init_game_state(void)
{
    GameState game_state = {
        .board = init_board(),
        .selected_piece = NULL,
        .selected_square = {-1, -1},
        .bit_position = -1,
    };

    return game_state;
}