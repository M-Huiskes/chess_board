#include "board.h"

#include <stdio.h>

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

static void print_board_from_bitboards(BoardState board)
{
    for (int row = 7; row >= 0; row--) {
        printf("%d |", row + 1);
        for (int file = 0; file < 8; file++) {
            int square = row * 8 + file;
            int piece_on_square = 0;

            uint64_t mask = (uint64_t) 1 << square;
            Piece piece;

            for (int i = 0; i < 12; i++) {
                if (i < 6) {
                    piece = board.white[i];
                } else {
                    piece = board.black[i - 6];
                }
                uint64_t piece_bb = *(piece.pos_bb);
                if (piece_bb & mask) {
                    printf("%c", piece.symbol);
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

Square get_square_by_user_input()
{
    char input[64];

    printf("Enter a square (e.g. e4): ");
    fgets(input, sizeof(input), stdin);

    if (strlen(input) < 2 || input[0] < 'a' || input[0] > 'h' ||
        input[1] < '1' || input[1] > '8') {
        printf("Invalid square.\n");
        return (Square){-1, -1};
    }

    char file = input[0];
    int row = input[1] - '0';
    
    Square selected_sq = (Square){row, file};

    printf("Selected square: %d %d \n", row, file);
    return selected_sq;
}

void play_terminal_game(BoardState board)
{
    int running = 1;
    int needs_redraw = 1;

    while (running) {
        if (needs_redraw) {
            print_board_from_bitboards(board);
            needs_redraw = 0;
        }

        Square input_sq = get_square_by_user_input();
    }
}