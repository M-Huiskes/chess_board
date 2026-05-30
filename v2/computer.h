#ifndef COMPUTER_H
#define COMPUTER_H

typedef struct GameState GameState;

typedef struct ComputerMove {
    int from;
    int to;
    char piece;
    char promote_to;
} ComputerMove;

int minimax(GameState *game_state, int depth, int is_maxing);
#endif