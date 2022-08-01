#include <SDL2/SDL.h>

int SCC(int ret)
{
    if (ret < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return (3);
    }
    return (0);
}

void *SCP(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}
