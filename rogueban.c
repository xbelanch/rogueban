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

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define PURPLE (u8)(0xff), (u8)(0x0), (u8)(0xff), (u8)(0xff)
#define RED (u8)(0xff), (u8)(0x0), (u8)(0x00), (u8)(0xff)
#define BLACK (u8)(0x0), (u8)(0x0), (u8)(0x0), (u8)(0xff)
#define WHITE (u8)(0xff), (u8)(0xff), (u8)(0xff), (u8)(0xff)


static int SCC(int ret)
{
    if (ret < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return (3);
    }
    return (0);
}

static void *SCP(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

void render_screen(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect dest)
{
    SCC(SDL_SetRenderTarget(renderer, texture));
    SCC(SDL_SetRenderDrawColor(renderer, PURPLE));
    SCC(SDL_RenderClear(renderer));

    // Render simple rectangle
    SDL_RenderDrawRect(renderer, &dest);
    SDL_SetRenderDrawColor(renderer, BLACK);
    SDL_RenderFillRect(renderer, &dest);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

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

    // Create a simple texture
    SDL_Rect dest = { .w = 16, .h = 16, .x = 0, .y = 0 };
    SDL_Texture *texture = SCP(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT));

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

        // Playing with a simple animated rect
        dest.x += dest.w;
        if (dest.x >= SCREEN_WIDTH) {
            dest.x = 0;
            dest.y += dest.h;
        }
        if (dest.y >= SCREEN_HEIGHT) {
            dest.y = 0;
        }

        render_screen(renderer, texture, dest);
        SDL_Delay(250);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
	texture = NULL;
	window = NULL;
	renderer = NULL;
    SDL_Quit();
    return (0);
}
