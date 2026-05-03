#include "bit_utils.h"
#include "board.h"
#include "state.h"

#include <stdio.h>
#include <string.h>

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

static void print_board_from_bitboards(BoardState *board,
                                       uint64_t possible_moves)
{
    for (int row = 7; row >= 0; row--) {
        printf("%d |", row + 1);
        for (int file = 0; file < 8; file++) {
            int square = row * 8 + file;
            int piece_on_square = 0;

            uint64_t mask = (uint64_t) 1 << square;

            for (int i = 0; i < 12; i++) {
                Piece *piece = get_piece_by_index(i, board);
                uint64_t piece_bb = piece->pos_bb;

                if (piece_bb & mask) {
                    printf("%c", piece->symbol);
                    piece_on_square = 1;
                    break;
                }
            }

            if (!(piece_on_square)) {
                if (possible_moves & mask) {
                    printf("x");
                } else {
                    printf(".");
                }
            }
        }
        printf("\n");
    }
    printf("   --------\n");
    printf("   abcdefgh\n");
}

static Square get_square_by_user_input(int first_call)
{
    char input[64];

    if (first_call) {
        printf("Select a piece by entering a square (e.g. e4): ");
    } else {
        printf("Select the output square (e.g. e4): ");
    }

    fgets(input, sizeof(input), stdin);

    if (strlen(input) > 2 && input[2] != '\n' && input[2] != '\0') {
        printf("Too many input characters, only type two (e.g. e4)\n");
        return get_square_by_user_input(first_call);
    }

    if (strlen(input) < 2 || input[0] < 'a' || input[0] > 'h' ||
        input[1] < '1' || input[1] > '8') {
        printf("Invalid square.\n");
        return get_square_by_user_input(first_call);
    }

    int file = input[0] - 'a';
    int row = input[1] - '0' - 1;

    Square selected_sq = (Square){file, row};

    printf("Selected file: %d, row %d \n", file, row);
    return selected_sq;
}

void get_input_piece(GameState *game_state)
{
    Square input_sq = get_square_by_user_input(1);
    Piece *input_piece = get_piece_by_square(&input_sq, &(game_state->board));

    if (input_piece == NULL) {
        printf("Not a piece on this position, try again\n");
        return get_input_piece(game_state);
    }

    printf("You selected %c \n", input_piece->symbol);
    game_state->selected_piece = input_piece;
    game_state->bit_position = position_from_square(&input_sq);
    game_state->selected_square = input_sq;
}

Square get_user_output_square(uint64_t possible_moves)
{
    Square output_sq = get_square_by_user_input(0);
    int output_position = position_from_square(&output_sq);

    if (!(is_bit_set(possible_moves, output_position))) {
        printf("Not a viable move, try again\n");
        return get_user_output_square(possible_moves);
    }

    return output_sq;
}

void play_terminal_game(GameState *game_state)
{
    int running = 1;
    int needs_redraw = 1;
    uint64_t possible_moves = (uint64_t) 0;
    print_board_from_bitboards(&(game_state->board), possible_moves);

    while (running) {
        get_input_piece(game_state);
        possible_moves = find_possible_moves(game_state);
        print_board_from_bitboards(&(game_state->board), possible_moves);
        Square output_sq = get_user_output_square(possible_moves);
    }
}