#include "draw_interface.h"

#include "debug.h"

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

typedef int (*draw_luafunction)(struct draw_data *data, lua_State *L);

int drawfunction_wrapper(lua_State *L) {
    void *d = lua_touserdata(L, lua_upvalueindex(1));
    struct draw_data *data = (struct draw_data *)d;

    void *f = lua_touserdata(L, lua_upvalueindex(2));
    draw_luafunction func = (draw_luafunction)f;

    return func(data, L);
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

    GLuint program = get_integer_arg(L);

    glUseProgram(program);

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

int draw_lua_SDL_GL_SwapWindow(struct draw_data *data, lua_State *L) {
    (void)L;

    SDL_GL_SwapWindow(data->window);

    return 0;
}

#define REGISTER_FUNC(func) \
    register_drawfunction(L, draw_lua_ ## func, draw, "draw_" #func)

#define REGISTER_CONST(val) \
    register_drawconst(L, val, "draw_" #val)

void draw_interface_register(lua_State *L, struct draw_data *draw) {
    REGISTER_FUNC(glClearColor);
    REGISTER_FUNC(glClear);
    REGISTER_FUNC(glUseProgram);
    REGISTER_FUNC(glDrawArrays);
    REGISTER_FUNC(glEnableVertexAttribArray);
    REGISTER_FUNC(glDisableVertexAttribArray);
    REGISTER_FUNC(glVertexAttribPointer);
    REGISTER_FUNC(SDL_GL_SwapWindow);

    REGISTER_CONST(GL_COLOR_BUFFER_BIT);
    REGISTER_CONST(GL_DEPTH_BUFFER_BIT);
    REGISTER_CONST(GL_STENCIL_BUFFER_BIT);

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

    REGISTER_CONST(GL_TRUE);
    REGISTER_CONST(GL_FALSE);

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
}
