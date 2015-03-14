#include "lua.h"

#include "draw_interface.h"

void print_lua_error(const char *prefix, lua_State *L) {
    size_t err_len;
    const char *err = lua_tolstring(L, -1, &err_len);

    fprintf(stderr, "%s: %s\n", prefix, err);
}

int lua_setup(struct lua_data *data, struct draw_data *draw, const char *main_file) {
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Error making lua state");
        return 1;
    }
    luaL_openlibs(L);

    draw_interface_register(L, draw);

    int load_error = luaL_loadfile(L, main_file);
    if (load_error != LUA_OK) {
        print_lua_error("Error loading", L);
        lua_close(L);
        return 1;
    }

    int run_error = lua_pcall(L, 0, 0, 0);
    if (run_error != LUA_OK) {
        print_lua_error("Error running", L);
        lua_close(L);
        return 1;
    }

    lua_State *L2 = lua_newthread(L);

    data->renderL = L;
    data->updateL = L2;

    return 0;
}

void lua_cleanup(struct lua_data *data) {
    lua_close(data->renderL);
}

void lua_cleanup_wrapper(void *d) {
    struct lua_data *data = (struct lua_data *)d;
    lua_cleanup(data);
}
