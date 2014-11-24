#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>

struct draw_data {
    SDL_Window *window;
};

int draw_setup(struct draw_data *);
void draw_cleanup(struct draw_data *);
void draw_cleanup_wrapper(void *);

#endif
