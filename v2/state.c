#include "state.h"

#include "board.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

MoveRecord encode_move(int from, int to, int flags, char captured_piece)
{
    MoveRecord move_record = {
        .move = (uint16_t) ((flags << 12) | (to << 6) | from),
        .captured_piece = captured_piece,
    };
    return move_record;
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

void push_move(MoveHistory *history, MoveRecord move)
{
    if (history->count == history->capacity) {
        history->capacity *= 2;
        history->moves = realloc(history->moves, history->capacity);
    }
    history->moves[history->count++] = move;
}

MoveHistory init_move_history(void)
{
    MoveHistory h;
    h.count = 0;
    h.capacity = 128;
    h.moves = malloc(h.capacity * sizeof(MoveRecord));

    return h;
}

GameState init_game_state(void)
{
    GameState game_state = {.board = init_board(),
                            .selected_piece = NULL,
                            .selected_square = {-1, -1},
                            .bit_position = -1,
                            .move_history = init_move_history(),
                            .last_moved_piece = '0',
                            .en_passant_possible = 0,
                            .promote_to = '0',
                            .output_position = 0,
                            .possible_moves = (uint64_t) 0,
                            .awaiting_promotion = 0,
                            .check_info =
                                (CheckInfo){
                                    .is_check = 0,
                                    .is_double_check = 0,
                                    .check_by = '0',
                                    .position_check = 0,
                                },
                            .computer_move = (ComputerMove){
                                .from = -1,
                                .to = -1,
                                .piece = '0',
                            }};

    return game_state;
}

OldState write_old_state(GameState *game_state)
{
    return (OldState){
        .last_moved_piece = game_state->last_moved_piece,
        .en_passant_possible = game_state->en_passant_possible,
        .promote_to = game_state->promote_to,
        .possible_moves = game_state->possible_moves,
        .awaiting_promotion = game_state->awaiting_promotion,
        .check_info = game_state->check_info,
    };
};