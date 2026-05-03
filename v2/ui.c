#include "ui.h"

#include "state.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SDL_Renderer *get_window_renderer()
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

// void get_selected_square() {}

void render_board(SDL_Renderer *renderer, GameState *game_state,
                  uint64_t possible_moves)
{
    for (int row = 7; row >= 0; row--) {
        printf("Row: %d\n", row);
        for (int file = 0; file < 8; file++) {
            printf("file: %d\n", file);
            SDL_Rect rect = {file * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE,
                             SQUARE_SIZE};
            if ((row + file) % 2 == 1) {
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
            }
            SDL_RenderFillRect(renderer, &rect);
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
            }
        }

        if (redraw_board) {
            render_board(renderer, game_state, possible_moves);
            redraw_board = 0;
        }
    }

    exit(EXIT_SUCCESS);
}