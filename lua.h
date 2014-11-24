#ifndef LUA_H
#define LUA_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

struct lua_data {
    lua_State *renderL;
    lua_State *updateL;
};

int lua_setup(struct lua_data *);
void lua_cleanup(struct lua_data *);
void lua_cleanup_wrapper(void *);

#endif
