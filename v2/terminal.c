#include "board.h"

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

static void print_board_from_bitboards(BoardState *board)
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
                printf(".");
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
    
    if (strlen(input) > 2 && input[2] != '\n' && input[2] != '\0' ) {
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

Piece *get_input_piece(BoardState *board)
{
    Square input_sq = get_square_by_user_input(1);
    Piece *input_piece = get_piece_by_square(&input_sq, board);

    if (input_piece == NULL) {
        printf("Not a piece on this position, try again\n");
        return get_input_piece(board);
    }

    printf("You selected %c \n", input_piece->symbol);
    return input_piece;
}

void play_terminal_game(BoardState *board)
{
    int running = 1;
    int needs_redraw = 1;

    while (running) {
        print_board_from_bitboards(board);

        Piece *input_piece = get_input_piece(board);
        // Square output_sq = get_square_by_user_input(0);
    }
}