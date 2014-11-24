#include "draw.h"

void print_sdl_error(const char *prefix) {
    const char *error = SDL_GetError();

    fprintf(stderr, "%s: %s", prefix, error);
}

int draw_setup(struct draw_data *data) {
    int err;
    if ((err = SDL_Init(SDL_INIT_VIDEO)) < 0) {
        print_sdl_error("Error initializing SDL");
        return 1;
    }

    if ((err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) < 0) {
        print_sdl_error("Error setting SDL context major version");
        SDL_Quit();
        return 1;
    }
    if ((err = SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) < 0) {
        print_sdl_error("Error setting SDL context minor version");
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Test window", 0, 0, 800, 600,
                                          SDL_WINDOW_OPENGL);

    if (!window) {
        print_sdl_error("Error creating SDL window");
        SDL_Quit();
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!context) {
        print_sdl_error("Error getting SDL OpenGL context");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int major, minor;
    if ((err = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major)) < 0) {
        print_sdl_error("Error getting SDL context major version");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if ((err = SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor)) < 0) {
        print_sdl_error("Error getting SDL context minor version");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    printf("Got OpenGL Version: %d.%d\n", major, minor);

    data->window = window;

    return 0;
}

void draw_cleanup(struct draw_data *data) {
    SDL_DestroyWindow(data->window);
    SDL_Quit();
}

void draw_cleanup_wrapper(void *d) {
    struct draw_data *data = (struct draw_data *)d;
    draw_cleanup(data);
}
