#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
    int status_code;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
} RenderContext;