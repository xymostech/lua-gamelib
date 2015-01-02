#ifndef DRAW_H
#define DRAW_H

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_video.h>

struct draw_data {
    SDL_Window *window;
    SDL_GLContext context;
};

int draw_setup(struct draw_data *);
void draw_cleanup(struct draw_data *);
void draw_cleanup_wrapper(void *);

#endif
