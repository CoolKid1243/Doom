#include <SDL2/SDL.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>

#undef main

#define screenW 1152
#define screenH 758

SDL_Renderer* renderer;

int ShouldQuit(SDL_Event event) {
    if(event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) return 1; // close the window if we press 'escape'

    return 0;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* mainWin = SDL_CreateWindow(
        "knock-off doom",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenW, screenH,
        SDL_WINDOW_SHOWN
    );
 
    renderer = SDL_CreateRenderer(mainWin, 0, SDL_RENDERER_SOFTWARE);

    int loop = 1;
    SDL_Event event;
    while (loop) {
        SDL_PollEvent(&event);

        if (ShouldQuit(event)) break;
    }
 
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWin);
    SDL_Quit();
    return 0;
}