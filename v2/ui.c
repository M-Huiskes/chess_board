#include "ui.h"

#include "board.h"
#include "state.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_Renderer static *get_window_renderer()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n",
               IMG_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_Window *window =
        SDL_CreateWindow("Chessboard", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_SHOWN);

    return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void static process_user_input(SDL_Event event, GameState *game_state)
{
    int file = event.button.x / SQUARE_SIZE;
    int row = 7 - (event.button.y / SQUARE_SIZE);

    printf("File selected %d, row selected %d \n", file, row);

    if (game_state->selected_square.row == row &&
        game_state->selected_square.file == file) {
        // If same square is selected -> unselect square
        game_state->selected_square = (Square){-1, -1};
        game_state->selected_piece = NULL;
        game_state->bit_position = -1;

        return;
    }
    Square selected_sq = (Square){file, row};
    int position = position_from_square(&selected_sq);

    game_state->selected_square = selected_sq;
    game_state->bit_position = position;
    game_state->selected_piece =
        get_piece_by_square(&selected_sq, &(game_state->board));
}

const char *get_image_path(char symbol)
{
    switch (symbol) {
    case 'P':
        return "images/pawn_w.png";
    case 'p':
        return "images/pawn_b.png";
    case 'R':
        return "images/rook_w.png";
    case 'r':
        return "images/rook_b.png";
    case 'N':
        return "images/knight_w.png";
    case 'n':
        return "images/knight_b.png";
    case 'B':
        return "images/bishop_w.png";
    case 'b':
        return "images/bishop_b.png";
    case 'Q':
        return "images/queen_w.png";
    case 'q':
        return "images/queen_b.png";
    case 'K':
        return "images/king_w.png";
    case 'k':
        return "images/king_b.png";
    default:
        return NULL;
    }
}

void static bitboards_to_board(char board_repr[8][8], BoardState *board_state)
{
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            uint64_t mask = (uint64_t) 1 << sq;
            char piece_symbol = 0;

            for (int i = 0; i < 12; i++) {
                Piece *piece = get_piece_by_index(i, board_state);
                if (piece->pos_bb & mask) {
                    piece_symbol = piece->symbol;
                }
            }
            board_repr[rank][file] = piece_symbol;
        }
    }
}

void static render_piece(char symbol, SDL_Renderer *renderer, int file, int row)
{
    SDL_Texture *tex = NULL;
    SDL_Surface *surface = IMG_Load(get_image_path(symbol));
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return;
    }
    tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (tex) {
        SDL_Rect pieceRect = {file * SQUARE_SIZE, row * SQUARE_SIZE,
                              SQUARE_SIZE, SQUARE_SIZE};
        SDL_RenderCopy(renderer, tex, NULL, &pieceRect);
    }
}

void static render_board(SDL_Renderer *renderer, GameState *game_state,
                         uint64_t possible_moves)
{
    char board_repr[8][8];
    bitboards_to_board(board_repr, &(game_state->board));

    for (int row = 7; row >= 0; row--) {
        for (int file = 0; file < 8; file++) {
            int render_row = 7 - row; // flip for SDL coordinate system
            SDL_Rect rect = {file * SQUARE_SIZE, render_row * SQUARE_SIZE,
                             SQUARE_SIZE, SQUARE_SIZE};
            if (game_state->selected_square.file == file &&
                game_state->selected_square.row == row &&
                game_state->selected_piece != NULL) {
                SDL_SetRenderDrawColor(renderer, 60, 80, 50, 180);
            } else if ((row + file) % 2 == 1) {
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
            }
            SDL_RenderFillRect(renderer, &rect);

            if (board_repr[render_row][file] != 0) {
                render_piece(board_repr[row][file], renderer, file, render_row);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void play_ui_game(GameState *game_state)
{
    int running = 1;
    int redraw_board = 1;
    uint64_t possible_moves = (uint64_t) 0;

    SDL_Renderer *renderer = get_window_renderer();
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                process_user_input(event, game_state);
                redraw_board = 1;
            }
        }

        if (redraw_board) {
            printf("Selected square row: %d, file: %d\n",
                   game_state->selected_square.row,
                   game_state->selected_square.file);
            render_board(renderer, game_state, possible_moves);
            redraw_board = 0;
        }
    }

    exit(EXIT_SUCCESS);
}