#include "draw.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>

void print_sdl_error(const char *prefix) {
    const char *error = SDL_GetError();

    fprintf(stderr, "%s: %s", prefix, error);
}

off_t get_file_size(FILE *f) {
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_END);
    off_t size = ftello(f);
    // TODO(emily): error checking
    fseeko(f, 0, SEEK_SET);

    return size;
}

GLuint create_shader(GLenum shader_type, const char *file_name) {
    GLuint shader = glCreateShader(shader_type);

    // TODO(emily): error checking
    FILE *f = fopen(file_name, "rb");
    off_t file_size = get_file_size(f);

    // TODO(emily): error checking
    GLchar *data = malloc(file_size);
    // TODO(emily): error checking
    fread(data, 1, file_size, f);

    glShaderSource(shader, 1, (const GLchar * const *)&data, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        fprintf(stderr, "shader bad: %s\n", file_name);
        // TODO(emily): error checking
    }

    free(data);

    return shader;
}

GLuint create_program(const char *vertex_file, const char *fragment_file) {
    // TODO(emily): error checking
    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_file);
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_file);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        // TODO(emily): error checking
        fprintf(stderr, "Bad linking");
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

GLuint create_vertex_buffer(const float *vertices, int size) {
    GLuint buffer_object = 0;

    glGenBuffers(1, &buffer_object);

    glBindBuffer(GL_ARRAY_BUFFER, buffer_object);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return buffer_object;
}

GLuint create_vertex_array() {
    GLuint vertex_array = 0;

    glGenVertexArrays(1, &vertex_array);

    return vertex_array;
}

const float vertex_positions[] = {
    0.75f, 0.75f, 0.0f, 1.0f,
    0.75f, -0.75f, 0.0f, 1.0f,
    -0.75f, -0.75f, 0.0f, 1.0f,
    1.0f,    0.0f, 0.0f, 1.0f,
    0.0f,    1.0f, 0.0f, 1.0f,
    0.0f,    0.0f, 1.0f, 1.0f,
};

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

    GLuint program = create_program("vertex.shader", "fragment.shader");
    GLuint vertex_buffer = create_vertex_buffer(
        vertex_positions, sizeof(vertex_positions));
    GLuint vertex_array = create_vertex_array();

    glBindVertexArray(vertex_array);

    data->window = window;
    data->context = context;

    data->program = program;
    data->vertex_buffer = vertex_buffer;
    data->vertex_array = vertex_array;

    return 0;
}

void draw_cleanup(struct draw_data *data) {
    // TODO(emily): cleanup gl stuff

    SDL_GL_DeleteContext(data->context);

    SDL_DestroyWindow(data->window);
    SDL_Quit();

    memset(data, 0x0, sizeof(*data));
}

void draw_cleanup_wrapper(void *d) {
    struct draw_data *data = (struct draw_data *)d;
    draw_cleanup(data);
}

void draw_draw(struct draw_data *data) {
    glUseProgram(data->program);

    glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
}
