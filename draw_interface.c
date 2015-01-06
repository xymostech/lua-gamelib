#include "draw_interface.h"

#include "debug.h"
#include "util.h"

lua_Integer get_integer_arg(lua_State *L) {
    // TODO(emily): error checking
    lua_Integer x = lua_tointeger(L, 1);
    lua_remove(L, 1);
    return x;
}

lua_Number get_float_arg(lua_State *L) {
    // TODO(emily): error checking
    lua_Number x = lua_tonumber(L, 1);
    lua_remove(L, 1);
    return x;
}

const char *get_string_arg(lua_State *L) {
    // TODO(emily): error checking
    const char *str = lua_tostring(L, 1);
    lua_remove(L, 1);
    return str;
}

lua_Integer get_userdata_arg(lua_State *L) {
    // TODO(emily): error checking
    lua_Integer data = (lua_Integer)lua_touserdata(L, 1);
    lua_remove(L, 1);
    return data;
}

lua_Integer get_lua_len(lua_State *L, int index) {
    lua_len(L, index);
    lua_Integer len = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return len;
}

#define STRERROR_HELPER(error) \
    case error: return #error; break

const char *gl_strerror(GLenum err) {
    switch (err) {
        STRERROR_HELPER(GL_NO_ERROR);
        STRERROR_HELPER(GL_INVALID_ENUM);
        STRERROR_HELPER(GL_INVALID_VALUE);
        STRERROR_HELPER(GL_INVALID_OPERATION);
        STRERROR_HELPER(GL_INVALID_FRAMEBUFFER_OPERATION);
        STRERROR_HELPER(GL_OUT_OF_MEMORY);
        STRERROR_HELPER(GL_STACK_UNDERFLOW);
        STRERROR_HELPER(GL_STACK_OVERFLOW);
    }

    return "Invalid error";
}

typedef int (*draw_luafunction)(struct draw_data *data, lua_State *L);

int drawfunction_wrapper(lua_State *L) {
    void *d = lua_touserdata(L, lua_upvalueindex(1));
    struct draw_data *data = (struct draw_data *)d;

    void *f = lua_touserdata(L, lua_upvalueindex(2));
    draw_luafunction func = (draw_luafunction)f;

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("Had error: %d (%s) before calling function\n", err, gl_strerror(err));
    }

    int ret = func(data, L);

    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("Got error: %d (%s) during function\n", err, gl_strerror(err));
    }

    return ret;
}

void register_drawfunction(lua_State *L, draw_luafunction func,
                           struct draw_data *data, const char *name) {
    lua_pushlightuserdata(L, (void*)data);
    lua_pushlightuserdata(L, (void*)func);

    lua_pushcclosure(L, drawfunction_wrapper, 2);

    lua_setglobal(L, name);
}

void register_drawconst(lua_State *L, lua_Integer val, const char *name) {
    lua_pushinteger(L, val);
    lua_setglobal(L, name);
    debugp("Setting %s to %ld", name, val);
}

int draw_lua_glClearColor(struct draw_data *data, lua_State *L) {
    (void)data;

    GLfloat r = get_float_arg(L);
    GLfloat g = get_float_arg(L);
    GLfloat b = get_float_arg(L);
    GLfloat a = get_float_arg(L);

    glClearColor(r, g, b, a);

    return 0;
}

int draw_lua_glClear(struct draw_data *data, lua_State *L) {
    (void)data;

    GLbitfield mask = get_integer_arg(L);

    glClear(mask);

    return 0;
}

int draw_lua_glUseProgram(struct draw_data *data, lua_State *L) {
    (void)data;

    if (lua_isnil(L, 1)) {
        glUseProgram(0);
    } else {
        GLuint program = (GLuint)lua_touserdata(L, 1);

        glUseProgram(program);
    }

    lua_pop(L, 1);

    return 0;
}

int draw_lua_glDrawArrays(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum mode = get_integer_arg(L);
    GLint first = get_integer_arg(L);
    GLsizei count = get_integer_arg(L);

    glDrawArrays(mode, first, count);

    return 0;
}

int draw_lua_glEnableVertexAttribArray(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint index = get_integer_arg(L);

    glEnableVertexAttribArray(index);

    return 0;
}

int draw_lua_glDisableVertexAttribArray(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint index = get_integer_arg(L);

    glDisableVertexAttribArray(index);

    return 0;
}

int draw_lua_glVertexAttribPointer(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint index = get_integer_arg(L);
    GLint size = get_integer_arg(L);
    GLenum type = get_integer_arg(L);
    GLboolean normalized = get_integer_arg(L);
    GLsizei stride = get_integer_arg(L);
    GLvoid *pointer = (GLvoid *)get_integer_arg(L);

    glVertexAttribPointer(index, size, type, normalized, stride, pointer);

    return 0;
}

int draw_lua_CreateShaderFromFile(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum shader_type = get_integer_arg(L);
    const char *file_name = get_string_arg(L);

    GLchar *file_data = read_whole_file(file_name);

    GLuint shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, (const GLchar * const *)&file_data, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        fprintf(stderr, "shader bad: %s\n", file_name);
        // TODO(emily): error checking
    }

    free(file_data);

    // TODO(emily): Offset this so it doesn't test equal to other OpenGL types.
    lua_pushlightuserdata(L, (void*)(intptr_t)shader);

    return 1;
}

int draw_lua_glDeleteShader(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint shader = get_integer_arg(L);

    glDeleteShader(shader);

    return 0;
}

int draw_lua_CreateProgramFromShaders(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint program = glCreateProgram();

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        // TODO(emily): check if the indices are integers?
        GLuint shader = (GLuint)lua_touserdata(L, -1);
        debugp("Attaching shader %d", shader);

        glAttachShader(program, shader);

        lua_pop(L, 1);
    }

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        // TODO(emily): error checking
        fprintf(stderr, "Bad linking");
    }

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        GLuint shader = (GLuint)lua_touserdata(L, -1);
        debugp("Detaching shader %d", shader);

        glDetachShader(program, shader);

        lua_pop(L, 1);
    }

    // Pop table
    lua_pop(L, 1);

    lua_pushlightuserdata(L, (void*)(intptr_t)program);

    return 1;
}

int draw_lua_glDeleteProgram(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint program = get_integer_arg(L);

    glDeleteProgram(program);

    return 0;
}

int draw_lua_CreateBufferObject(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint buffer_object;

    glGenBuffers(1, &buffer_object);

    lua_pushlightuserdata(L, (void*)(intptr_t)buffer_object);

    return 1;
}

int draw_lua_DeleteBufferObject(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint buffer = get_userdata_arg(L);

    glDeleteBuffers(1, &buffer);

    return 1;
}

int draw_lua_glBindBuffer(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum target = get_integer_arg(L);

    if (lua_isnil(L, 1)) {
        debugp("Unbinding buffer");
        glBindBuffer(target, 0);
    } else {
        GLuint buffer = (GLuint)lua_touserdata(L, 1);
        debugp("Binding buffer %d", buffer);

        glBindBuffer(target, buffer);
    }

    lua_pop(L, 1);

    return 0;
}

int draw_lua_glBufferData(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum target = get_integer_arg(L);
    GLsizeiptr size = get_integer_arg(L);
    GLenum usage = get_integer_arg(L);

    glBufferData(target, size, NULL, usage);

    return 0;
}

int draw_lua_BufferSubFloatData(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum target = get_integer_arg(L);
    GLintptr offset = get_integer_arg(L);

    lua_len(L, 1);
    lua_Integer count = lua_tointeger(L, -1);
    lua_pop(L, 1);

    double *buffer_data = malloc(count * sizeof(*buffer_data));

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        if (lua_isnumber(L, -2)) {
            buffer_data[lua_tointeger(L, -2) - 1] = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);
    }

    glBufferSubData(target, offset, count * sizeof(*buffer_data), (void*)buffer_data);

    free(buffer_data);

    return 0;
}

int draw_lua_CreateVertexArray(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint vertex_array;

    glGenVertexArrays(1, &vertex_array);

    lua_pushlightuserdata(L, (void*)(intptr_t)vertex_array);

    return 1;
}

int draw_lua_DeleteVertexArray(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint vertex_array = get_userdata_arg(L);

    glDeleteVertexArrays(1, &vertex_array);

    return 0;
}

int draw_lua_glBindVertexArray(struct draw_data *data, lua_State *L) {
    (void)data;

    if (lua_isnil(L, 1)) {
        glBindVertexArray(0);
    } else {
        GLuint vertex_array = (GLuint)lua_touserdata(L, 1);

        glBindVertexArray(vertex_array);
    }

    lua_pop(L, 1);

    return 0;
}

int draw_lua_glGetUniformLocation(struct draw_data *data, lua_State *L) {
    (void)data;

    GLuint program = get_userdata_arg(L);
    const char *name = get_string_arg(L);

    GLint uniform = glGetUniformLocation(program, name);

    lua_pushlightuserdata(L, (void*)(intptr_t)uniform);

    return 1;
}

void read_into_float_array(lua_State *L, int table_index, int count, float *data) {
    for (int i = 0; i < count; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, table_index);
        data[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
}

void (*floatUniformFunctions[4])(GLint, GLsizei, const GLfloat *) = {
    glUniform1fv,
    glUniform2fv,
    glUniform3fv,
    glUniform4fv
};

int draw_lua_glUniformFloat(struct draw_data *data, lua_State *L) {
    (void)data;

    GLint location = get_userdata_arg(L);
    lua_Integer len = get_lua_len(L, 1);

    GLfloat values[4];

    read_into_float_array(L, 1, len, values);

    lua_pop(L, 1);

    floatUniformFunctions[len - 1](location, 1, values);

    return 0;
}

void (*floatMatrixUniformFunctions[3][3])(GLint, GLsizei, GLboolean, const GLfloat *) = {
    {glUniformMatrix2fv,   glUniformMatrix2x3fv, glUniformMatrix2x4fv},
    {glUniformMatrix3x2fv, glUniformMatrix3fv,   glUniformMatrix3x4fv},
    {glUniformMatrix4x2fv, glUniformMatrix4x3fv, glUniformMatrix4fv  }
};

int draw_lua_glUniformMatrixFloat(struct draw_data *data, lua_State *L) {
    (void)data;

    GLint location = get_userdata_arg(L);
    lua_Integer width = get_integer_arg(L);
    lua_Integer height = get_integer_arg(L);

    GLfloat values[16];

    read_into_float_array(L, 1, width * height, values);

    for (int i = 0; i < 16; i++) {
        printf("%f ", values[i]);
    }
    printf("\n");

    lua_pop(L, 1);

    floatMatrixUniformFunctions[width - 2][height - 2](location, 1, GL_FALSE, values);

    return 0;
}

int draw_lua_glEnable(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum cap = get_integer_arg(L);

    glEnable(cap);

    return 0;
}

int draw_lua_glDisable(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum cap = get_integer_arg(L);

    glDisable(cap);

    return 0;
}

int draw_lua_glCullFace(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum mode = get_integer_arg(L);

    glCullFace(mode);

    return 0;
}

int draw_lua_glFrontFace(struct draw_data *data, lua_State *L) {
    (void)data;

    GLenum mode = get_integer_arg(L);

    glFrontFace(mode);

    return 0;
}

int draw_lua_SDL_GL_SwapWindow(struct draw_data *data, lua_State *L) {
    (void)L;

    SDL_GL_SwapWindow(data->window);

    return 0;
}

#define REGISTER_FUNC(func) \
    register_drawfunction(L, draw_lua_ ## func, draw, "draw_" #func)
// E.g. register_drawfunction(L, draw_lua_glClear, draw, "draw_glClear")

#define REGISTER_CONST(val) \
    register_drawconst(L, val, "draw_" #val)
// E.g. register_drawconst(L, GL_TRUE, "draw_GL_TRUE")

void draw_interface_register(lua_State *L, struct draw_data *draw) {
    /***** FUNCTIONS *****/
    // Clearing functions
    REGISTER_FUNC(glClearColor);
    REGISTER_FUNC(glClear);

    // Drawing functions
    REGISTER_FUNC(glDrawArrays);

    // Vertex Attrib Array functions
    REGISTER_FUNC(glEnableVertexAttribArray);
    REGISTER_FUNC(glDisableVertexAttribArray);
    REGISTER_FUNC(glVertexAttribPointer);

    // Shader and Program functions
    REGISTER_FUNC(CreateShaderFromFile);
    REGISTER_FUNC(glDeleteShader);
    REGISTER_FUNC(CreateProgramFromShaders);
    REGISTER_FUNC(glDeleteProgram);
    REGISTER_FUNC(glUseProgram);

    // Buffer object functions
    REGISTER_FUNC(CreateBufferObject);
    REGISTER_FUNC(DeleteBufferObject);
    REGISTER_FUNC(glBindBuffer);
    REGISTER_FUNC(glBufferData);
    REGISTER_FUNC(BufferSubFloatData);

    // Vertex array object functions
    REGISTER_FUNC(CreateVertexArray);
    REGISTER_FUNC(DeleteVertexArray);
    REGISTER_FUNC(glBindVertexArray);

    // Uniform functions
    REGISTER_FUNC(glGetUniformLocation);
    REGISTER_FUNC(glUniformFloat);
    REGISTER_FUNC(glUniformMatrixFloat);

    // Enable/Disable functions
    REGISTER_FUNC(glEnable);
    REGISTER_FUNC(glDisable);

    // Culling functions
    REGISTER_FUNC(glCullFace);
    REGISTER_FUNC(glFrontFace);

    // SDL functions
    REGISTER_FUNC(SDL_GL_SwapWindow);

    /***** CONSTANTS *****/
    // Flags for glClear
    REGISTER_CONST(GL_COLOR_BUFFER_BIT);
    REGISTER_CONST(GL_DEPTH_BUFFER_BIT);
    REGISTER_CONST(GL_STENCIL_BUFFER_BIT);

    // Drawing types for glDrawArrays
    REGISTER_CONST(GL_POINTS);
    REGISTER_CONST(GL_LINE_STRIP);
    REGISTER_CONST(GL_LINE_LOOP);
    REGISTER_CONST(GL_LINES);
    REGISTER_CONST(GL_LINE_STRIP_ADJACENCY);
    REGISTER_CONST(GL_LINES_ADJACENCY);
    REGISTER_CONST(GL_TRIANGLE_STRIP);
    REGISTER_CONST(GL_TRIANGLE_FAN);
    REGISTER_CONST(GL_TRIANGLES);
    REGISTER_CONST(GL_TRIANGLE_STRIP_ADJACENCY);
    REGISTER_CONST(GL_TRIANGLES_ADJACENCY);
    REGISTER_CONST(GL_PATCHES);

    // Booleans
    REGISTER_CONST(GL_TRUE);
    REGISTER_CONST(GL_FALSE);

    // Generic data types
    REGISTER_CONST(GL_BYTE);
    REGISTER_CONST(GL_UNSIGNED_BYTE);
    REGISTER_CONST(GL_SHORT);
    REGISTER_CONST(GL_UNSIGNED_SHORT);
    REGISTER_CONST(GL_INT);
    REGISTER_CONST(GL_UNSIGNED_INT);
    REGISTER_CONST(GL_HALF_FLOAT);
    REGISTER_CONST(GL_FLOAT);
    REGISTER_CONST(GL_DOUBLE);
    REGISTER_CONST(GL_FIXED);
    REGISTER_CONST(GL_INT_2_10_10_10_REV);
    REGISTER_CONST(GL_UNSIGNED_INT_2_10_10_10_REV);
    REGISTER_CONST(GL_UNSIGNED_INT_10F_11F_11F_REV);

    // Shader types
    REGISTER_CONST(GL_COMPUTE_SHADER);
    REGISTER_CONST(GL_VERTEX_SHADER);
    REGISTER_CONST(GL_TESS_CONTROL_SHADER);
    REGISTER_CONST(GL_TESS_EVALUATION_SHADER);
    REGISTER_CONST(GL_GEOMETRY_SHADER);
    REGISTER_CONST(GL_FRAGMENT_SHADER);

    // Buffer object types
    REGISTER_CONST(GL_ARRAY_BUFFER);
    REGISTER_CONST(GL_ATOMIC_COUNTER_BUFFER);
    REGISTER_CONST(GL_COPY_READ_BUFFER);
    REGISTER_CONST(GL_COPY_WRITE_BUFFER);
    REGISTER_CONST(GL_DISPATCH_INDIRECT_BUFFER);
    REGISTER_CONST(GL_DRAW_INDIRECT_BUFFER);
    REGISTER_CONST(GL_ELEMENT_ARRAY_BUFFER);
    REGISTER_CONST(GL_PIXEL_PACK_BUFFER);
    REGISTER_CONST(GL_PIXEL_UNPACK_BUFFER);
    REGISTER_CONST(GL_QUERY_BUFFER);
    REGISTER_CONST(GL_SHADER_STORAGE_BUFFER);
    REGISTER_CONST(GL_TEXTURE_BUFFER);
    REGISTER_CONST(GL_TRANSFORM_FEEDBACK_BUFFER);
    REGISTER_CONST(GL_UNIFORM_BUFFER);

    // Buffer accessing modes
    REGISTER_CONST(GL_STREAM_DRAW);
    REGISTER_CONST(GL_STREAM_READ);
    REGISTER_CONST(GL_STREAM_COPY);
    REGISTER_CONST(GL_STATIC_DRAW);
    REGISTER_CONST(GL_STATIC_READ);
    REGISTER_CONST(GL_STATIC_COPY);
    REGISTER_CONST(GL_DYNAMIC_DRAW);
    REGISTER_CONST(GL_DYNAMIC_READ);
    REGISTER_CONST(GL_DYNAMIC_COPY);

    // glEnable/glDisable capabilities
    REGISTER_CONST(GL_BLEND);
    REGISTER_CONST(GL_CLIP_DISTANCE0);
    REGISTER_CONST(GL_CLIP_DISTANCE1);
    REGISTER_CONST(GL_CLIP_DISTANCE2);
    REGISTER_CONST(GL_CLIP_DISTANCE3);
    REGISTER_CONST(GL_CLIP_DISTANCE4);
    REGISTER_CONST(GL_CLIP_DISTANCE5);
    REGISTER_CONST(GL_CLIP_DISTANCE6);
    REGISTER_CONST(GL_CLIP_DISTANCE7);
    REGISTER_CONST(GL_COLOR_LOGIC_OP);
    REGISTER_CONST(GL_CULL_FACE);
    REGISTER_CONST(GL_DEBUG_OUTPUT);
    REGISTER_CONST(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    REGISTER_CONST(GL_DEPTH_CLAMP);
    REGISTER_CONST(GL_DEPTH_TEST);
    REGISTER_CONST(GL_DITHER);
    REGISTER_CONST(GL_FRAMEBUFFER_SRGB);
    REGISTER_CONST(GL_LINE_SMOOTH);
    REGISTER_CONST(GL_MULTISAMPLE);
    REGISTER_CONST(GL_POLYGON_OFFSET_FILL);
    REGISTER_CONST(GL_POLYGON_OFFSET_LINE);
    REGISTER_CONST(GL_POLYGON_OFFSET_POINT);
    REGISTER_CONST(GL_POLYGON_SMOOTH);
    REGISTER_CONST(GL_PRIMITIVE_RESTART);
    REGISTER_CONST(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    REGISTER_CONST(GL_RASTERIZER_DISCARD);
    REGISTER_CONST(GL_SAMPLE_ALPHA_TO_COVERAGE);
    REGISTER_CONST(GL_SAMPLE_ALPHA_TO_ONE);
    REGISTER_CONST(GL_SAMPLE_COVERAGE);
    REGISTER_CONST(GL_SAMPLE_SHADING);
    REGISTER_CONST(GL_SAMPLE_MASK);
    REGISTER_CONST(GL_SCISSOR_TEST);
    REGISTER_CONST(GL_STENCIL_TEST);
    REGISTER_CONST(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    REGISTER_CONST(GL_PROGRAM_POINT_SIZE);

    // Cull face parameters
    REGISTER_CONST(GL_FRONT);
    REGISTER_CONST(GL_BACK);
    REGISTER_CONST(GL_FRONT_AND_BACK);

    // Polygon orientations
    REGISTER_CONST(GL_CW);
    REGISTER_CONST(GL_CCW);
}
