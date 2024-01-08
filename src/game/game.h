#ifndef GAME_H
#define GAME_H

#include "lua.hpp"

#include "core/window.h"
#include "game/world.h"

class Game
{
    static World world;

    // Gameplay Lua callbacks
    static int pauseWorld(lua_State*);
    static int loadWorldFile(lua_State*);
    static int loadEventFile(lua_State*);
    static int movePlayer(lua_State*);
public:
    void initialize(lua_State*);

    void update(lua_State*, float);

    void draw(lua_State*, Window&);
};

#endif // GAME_H
