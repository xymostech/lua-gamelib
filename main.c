#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "draw.h"
#include "debug.h"

// pthreads
#include <pthread.h>

int handle_lua_error(int val, const char *prefix, lua_State *L, int exit) {
    if (val != LUA_OK) {
        size_t err_len;
        const char *err = lua_tolstring(L, -1, &err_len);

        fprintf(stderr, "%s: %s\n", prefix, err);
        if (exit) {
            pthread_exit(NULL);
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

int handle_posix_error(int val, char *prefix, int exit) {
    if (val != 0) {
        fprintf(stderr, "%s: %s\n", prefix, strerror(val));
        if (exit) {
            pthread_exit(NULL);
        }
    }
    return val;
}

void cleanup_lua_state(void *data) {
    lua_State *L = (lua_State *)data;

    lua_close(L);
}

void cleanup_destroy_mutex(void *data) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)data;

    handle_posix_error(pthread_mutex_destroy(mutex),
                       "Error destroying mutex", 0);
}

void cleanup_unlock_mutex(void *data) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)data;

    pthread_mutex_unlock(mutex);
}

void cleanup_destroy_cond(void *data) {
    pthread_cond_t *condition = (pthread_cond_t *)data;

    pthread_cond_destroy(condition);
}

void cleanup_set_done(void *data) {
    int *done = (int *)data;

    *done = 1;
}

void startup_lua(lua_State **LL) {
    lua_State *L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Error making lua state");
        pthread_exit(NULL);
    }
    pthread_cleanup_push(cleanup_lua_state, L);

    luaL_openlibs(L);

    handle_lua_error(luaL_loadfile(L, "threading.lua"),
                     "Error loading", L, 1);

    handle_lua_error(lua_pcall(L, 0, 0, 0),
                     "Error running", L, 1);

    *LL = L;

    pthread_cleanup_pop(0);
}

struct thread_data {
    struct lua_data *lua_data;
    struct draw_data *draw_data;
};

void register_cfunction(lua_State *L, lua_CFunction func,
                        struct thread_data *d, const char *name) {
    lua_pushlightuserdata(L, (void *)d);
    lua_pushcclosure(L, func, 1);
    lua_setglobal(L, name);
}

int lua_draw_draw_wrapper(lua_State *L) {
    void *d = lua_touserdata(L, lua_upvalueindex(1));

    struct thread_data *data = (struct thread_data *)d;

    draw_draw(data->draw_data);

    return 0;
}

int update(lua_State *L) {
    debugp("Updating...");

    lua_getglobal(L, "update_thread_update");
    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "Error: update_thread_update isn't a function\n");
        return 1;
    }

    lua_insert(L, -2);
    if (handle_lua_error(lua_pcall(L, 1, 2, 0),
                         "Error calling update_thread_update", L, 0)) {
        return 1;
    }

    int done = lua_toboolean(L, -1);
    lua_pop(L, 1);

    debugp("Updated. Done: %d", done);

    return done;
}

void render(lua_State *L) {
    debugp("Rendering...");

    lua_getglobal(L, "render_thread_render");
    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "Error: render_thread_render isn't a function\n");
        pthread_exit(NULL);
    }

    lua_insert(L, -2);
    handle_lua_error(lua_pcall(L, 1, 0, 0),
                     "Error calling render_thread_render", L, 1);
}

void transfer(struct lua_data *lua_data) {
    lua_pushvalue(lua_data->updateL, -1);
    lua_xmove(lua_data->updateL, lua_data->renderL, 1);
}

void *update_thread(void *data) {
    struct thread_data *d = (struct thread_data *)data;

    int done = 0;

    while (!done) {
        done = update(d->lua_data->updateL);

        transfer(d->lua_data);

        render(d->lua_data->renderL);

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1) {
            switch (event.type) {
                case SDL_QUIT:
                    debugp("Got quit event");
                    done = 1;
                    break;

                default:
                    break;
            }
        }

        SDL_Delay(16);
    }

    pthread_exit(NULL);
}

int main() {
    int err;

    struct lua_data lua_data;
    struct draw_data draw_data;

    if ((err = lua_setup(&lua_data, &draw_data)) != 0) {
        pthread_exit(NULL);
    }
    pthread_cleanup_push(lua_cleanup_wrapper, &lua_data);

    if ((err = draw_setup(&draw_data)) != 0) {
        pthread_exit(NULL);
    }
    pthread_cleanup_push(draw_cleanup_wrapper, &draw_data);

    lua_getglobal(lua_data.renderL, "startup");
    if (!lua_isfunction(lua_data.renderL, -1)) {
        fprintf(stderr, "startup function not defined\n");
        pthread_exit(NULL);
    }
    handle_lua_error(lua_pcall(lua_data.renderL, 0, 1, 0),
                     "Error starting up main thread", lua_data.renderL, 1);
    lua_pushvalue(lua_data.renderL, -1);
    lua_xmove(lua_data.renderL, lua_data.updateL, 1);

    struct thread_data data;

    register_cfunction(lua_data.renderL, lua_draw_draw_wrapper, &data, "c_draw");

    data.lua_data = &lua_data;
    data.draw_data = &draw_data;

    update_thread(&data);

    pthread_cleanup_pop(1); // cleanup draw
    pthread_cleanup_pop(1); // cleanup lua
}
