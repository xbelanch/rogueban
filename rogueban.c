#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef scc
void scc(int ret)
{
    if (ret < 0) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void *scp(void *ptr)
{
    if (ptr == NULL) {
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv[0];

    fprintf(stdout, "Rogueban\n");
    scc(SDL_Init(SDL_INIT_VIDEO));
    return (0);
}
