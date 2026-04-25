#include "board.h"
#include "pieces.h"
#include "terminal.h"

#include <stdio.h>
#include <string.h>

void init_team_state(Piece team[6], char color)
{
    uint64_t *start_positions[] = {
        color == 'w' ? &START_WHITE_PAWNS : &START_BLACK_PAWNS,
        color == 'w' ? &START_WHITE_ROOKS : &START_BLACK_ROOKS,
        color == 'w' ? &START_WHITE_KNIGHTS : &START_BLACK_KNIGHTS,
        color == 'w' ? &START_WHITE_BISHOPS : &START_BLACK_BISHOPS,
        color == 'w' ? &START_WHITE_KING : &START_BLACK_KING,
        color == 'w' ? &START_WHITE_QUEEN : &START_BLACK_QUEEN,
    };
    char *symbols;
    if (color == 'w') {
        symbols = "PRNBQK";
    } else {
        symbols = "prnbqk";
    }
    int values[] = {1, 3, 3, 5, 9, 0};

    for (int i = 0; i < 6; i++) {
        team[i] = (Piece){
            .color = color,
            .pos_bb = start_positions[i],
            .symbol = symbols[i],
            .value = values[i],
        };
    }
}

BoardState init_board(void)
{
    BoardState board;
    init_team_state(board.white, 'w');
    init_team_state(board.black, 'b');

    return board;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: ./chess <terminal|ui>\n");
        return 1;
    }

    BoardState board = init_board();

    if (strcmp(argv[1], "terminal") == 0) {
        play_terminal_game(board);
    } else if (strcmp(argv[1], "ui") == 0) {
        // start SDL2 UI-based game
        printf("Starting UI mode...\n");
    } else {
        printf("Unknown mode: %s\nUsage: ./chess <terminal|ui>\n", argv[1]);
        return 1;
    }

    return 0;
}