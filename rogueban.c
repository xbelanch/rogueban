#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t i32;
typedef int64_t i64;

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define PURPLE (u8)(0xff), (u8)(0x00), (u8)(0xff), (u8)(0xff)

static void SCC(int ret)
{
    if (ret < 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        exit(1);
    }
}

static void *SCP(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

void render_screen(SDL_Renderer *renderer, SDL_Window *window)
{
    SCC(SDL_SetRenderDrawColor(renderer, PURPLE));
    SCC(SDL_RenderClear(renderer));
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];

    // SDL2 init stuff
    SCC(SDL_Init(SDL_INIT_VIDEO));
    SDL_Window *window = SCP(SDL_CreateWindow("Rogueban", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0));
    SDL_Renderer *renderer = SCP(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Game loop
    bool quit = false;
    while (!quit) {
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    quit = true;
                    break;
                }
                default: break;
                }
            }
        }

        render_screen(renderer, window);
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return (0);
}
