#ifndef INCLUDE_SDL_HELPERS_H
#define INCLUDE_SDL_HELPERS_H
#include <SDL2/SDL.h>

int SCC(int ret);
void *SCP(void *ptr);
SDL_Surface *get_suface_from_file(const char *file_path);

#endif
