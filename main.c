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
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int other_is_finished;
    int done;
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

void print_state(const char *prefix, struct thread_data *d) {
    debugp("%s (other: %d, done: %d)\n", prefix, d->other_is_finished, d->done);
}

int update(lua_State *L) {
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

    return done;
}

void transfer(struct lua_data *lua_data) {
    lua_pushvalue(lua_data->updateL, -1);
    lua_xmove(lua_data->updateL, lua_data->renderL, 1);
}

void *update_thread(void *data) {
    struct thread_data *d = (struct thread_data *)data;

    int done = 0;
    pthread_cleanup_push(cleanup_set_done, &d->done);

    print_state("update: before loop", d);

    while (!done) {
        print_state("update: updating", d);
        done = update(d->lua_data->updateL);
        print_state("update: done updating", d);

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1) {
            switch (event.type) {
                case SDL_QUIT:
                    done = 1;
                    break;

                default:
                    break;
            }
        }

        print_state("update: before lock", d);
        handle_posix_error(pthread_mutex_lock(&d->mutex),
                           "Error locking mutex (update)", 1);
        pthread_cleanup_push(cleanup_unlock_mutex, &d->mutex);
        print_state("update: got lock", d);


        if (d->other_is_finished) {
            print_state("update: other was finished", d);
            d->other_is_finished = 0;

            print_state("update: transfer", d);
            d->done = done;
            transfer(d->lua_data);

            print_state("update: signalling", d);
            handle_posix_error(pthread_cond_signal(&d->condition),
                               "Error signalling", 1);
        } else {
            print_state("update: we finished first", d);
            d->other_is_finished = 1;

            print_state("update: waiting", d);
            handle_posix_error(pthread_cond_wait(&d->condition, &d->mutex),
                               "Error waiting on condition", 1);
            print_state("update: finished waiting", d);

            print_state("update: transfer", d);
            d->done = done;
            transfer(d->lua_data);

            print_state("update: signalling", d);
            handle_posix_error(pthread_cond_signal(&d->condition),
                               "Error signalling", 1);
        }

        print_state("update: unlocking", d);
        pthread_cleanup_pop(1);
    }

    print_state("update: done with loop", d);
    pthread_cleanup_pop(1);

    pthread_exit(NULL);
}

void render(lua_State *L) {
    lua_getglobal(L, "render_thread_render");
    if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "Error: render_thread_render isn't a function\n");
        pthread_exit(NULL);
    }

    lua_insert(L, -2);
    handle_lua_error(lua_pcall(L, 1, 0, 0),
                     "Error calling render_thread_render", L, 1);
}

void render_thread(struct thread_data *d) {
    print_state("render: before loop", d);

    while (!d->done) {
        print_state("render: rendering", d);
        render(d->lua_data->renderL);
        print_state("render: done with render", d);

        SDL_Delay(16);

        print_state("render: locking", d);
        handle_posix_error(pthread_mutex_lock(&d->mutex),
                           "Error locking mutex (render)", 1);
        pthread_cleanup_push(cleanup_unlock_mutex, &d->mutex);
        print_state("render: got lock", d);

        if (d->other_is_finished) {
            print_state("render: other was finished first", d);
            if (d->done) {
                break;
            }

            d->other_is_finished = 0;

            print_state("render: signalling", d);
            handle_posix_error(pthread_cond_signal(&d->condition),
                               "Error signalling", 1);
            print_state("render: now waiting", d);
            handle_posix_error(pthread_cond_wait(&d->condition, &d->mutex),
                               "Error waiting on condition", 1);
            print_state("render: done waiting", d);
        } else {
            print_state("render: we finished first", d);
            if (d->done) {
                break;
            }

            d->other_is_finished = 1;

            print_state("render: waiting", d);
            handle_posix_error(pthread_cond_wait(&d->condition, &d->mutex),
                               "Error waiting on condition", 1);
        }

        print_state("render: unlocking", d);
        pthread_cleanup_pop(1);
    }

    print_state("render: out of loop", d);
    pthread_exit(NULL);
}

int main() {
    int err;

    struct lua_data lua_data;
    if ((err = lua_setup(&lua_data)) != 0) {
        pthread_exit(NULL);
    }
    pthread_cleanup_push(lua_cleanup_wrapper, &lua_data);

    struct draw_data draw_data;
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

    handle_posix_error(pthread_mutex_init(&data.mutex, NULL),
                       "Error creating mutex", 1);
    pthread_cleanup_push(cleanup_destroy_mutex, &data.mutex);

    handle_posix_error(pthread_cond_init(&data.condition, NULL),
                       "Error creating condition", 1);
    pthread_cleanup_push(cleanup_destroy_cond, &data.condition);

    pthread_t update;
    handle_posix_error(
        pthread_create(&update, NULL, update_thread, (void*)&data),
        "Error creating thread", 1);

    render_thread(&data);

    pthread_cleanup_pop(1); // cleanup cond
    pthread_cleanup_pop(1); // cleanup mutex
    pthread_cleanup_pop(1); // cleanup draw
    pthread_cleanup_pop(1); // cleanup lua
}
