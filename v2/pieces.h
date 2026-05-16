#ifndef PIECES_H
#define PIECES_H

#define KING_ARRAY_INDEX 5

#define WHITE_LEFT_ROOK_START 0
#define WHITE_RIGHT_ROOK_START 7

#define BLACK_LEFT_ROOK_START 56
#define BLACK_RIGHT_ROOK_START 63

#define WHITE_KING_START 4
#define BLACK_KING_START 60

#include <stdint.h>

typedef struct GameState GameState;
typedef struct OldState OldState;

const extern uint64_t START_WHITE_PAWNS;
const extern uint64_t START_BLACK_PAWNS;

const extern uint64_t START_WHITE_ROOKS;
const extern uint64_t START_BLACK_ROOKS;

const extern uint64_t START_WHITE_KNIGHTS;
const extern uint64_t START_BLACK_KNIGHTS;

const extern uint64_t START_WHITE_BISHOPS;
const extern uint64_t START_BLACK_BISHOPS;

const extern uint64_t START_WHITE_QUEEN;
const extern uint64_t START_BLACK_QUEEN;

const extern uint64_t START_WHITE_KING_START;
const extern uint64_t START_BLACK_KING_START;

typedef struct {
    uint64_t pos_bb;
    char symbol;
    char color;
    int value;
} Piece;

typedef struct {
    int pinned_position;
    int pinner_position;
    int direction;
} PinnedInfo;

typedef struct {
    Piece pieces[6];
    uint64_t attack_map;
    PinnedInfo *pin_info;
    int count_pinned_pieces;
    int short_castle_allowed;
    int long_castle_allowed;
} TeamState;

void init_pieces(Piece team[6], char color);
uint64_t find_possible_moves(GameState *game_state, int attack_moves_only);
uint64_t validate_moves_in_check(GameState *game_state,
                                 uint64_t possible_moves);
int check_diag_move(int position, int next_pos);
int check_vertical_move(int next_pos);
int check_horizontal_move(int position, int next_pos);
TeamState *get_team_state_by_color(GameState *game_state, char color);

#endif