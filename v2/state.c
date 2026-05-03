#include "state.h"

#include "board.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

MoveHistory init_move_history(void)
{
    MoveHistory h;
    h.count = 0;
    h.capacity = 128;
    h.moves = malloc(h.capacity * sizeof(uint16_t));

    return h;
}

uint16_t encode_move(int from, int to, int flags)
{
    return (uint16_t) ((flags << 12) | (to << 6) | from);
}

int get_from(uint16_t move)
{
    // 0x3F is 0011 1111 bit mask, used to extract 6 lowest set bits
    return move & 0x3F;
}

int get_to(uint16_t move)
{
    // 0x3F is 0011 1111 bit mask, used to extract 6 lowest set bits
    return (move >> 6) & 0x3F;
}

int get_flags(uint16_t move)
{
    // 0x3F is 1111 bit mask, used to extract 4 lowest set bits
    return (move >> 12) & 0xF;
}

void push_move(MoveHistory *history, uint16_t move)
{
    if (history->count == history->capacity) {
        history->capacity *= 2;
        history->moves = realloc(history->moves, history->capacity);
    }
    history->moves[history->count++] = move;
}

GameState init_game_state(void)
{
    GameState game_state = {
        .board = init_board(),
        .selected_piece = NULL,
        .selected_square = {-1, -1},
        .bit_position = -1,
        .move_history = init_move_history(),
        .last_moved_piece = '0',
        .en_passant_possible = 0,
        .promote_to = '0',
    };

    return game_state;
}