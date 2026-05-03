#include "state.h"
#include "terminal.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: ./chess <terminal|ui>\n");
        return 1;
    }

    GameState game_state = init_game_state();

    if (strcmp(argv[1], "terminal") == 0) {
        play_terminal_game(&game_state);
    } else if (strcmp(argv[1], "ui") == 0) {
        printf("Starting UI mode...\n");
    } else {
        printf("Unknown mode: %s\nUsage: ./chess <terminal|ui>\n", argv[1]);
        return 1;
    }

    return 0;
}