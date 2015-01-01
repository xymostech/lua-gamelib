#ifndef DRAW_INTERFACE_H
#define DRAW_INTERFACE_H

#include "lua.h"
#include "draw.h"

void draw_interface_register(lua_State *, struct draw_data *);

#endif
