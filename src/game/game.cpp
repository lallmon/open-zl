#include "game.h"

#include "game/worldloader.h"

World Game::world;

int Game::pauseWorld(lua_State *ctx)
{
    bool gopause = lua_toboolean(ctx, 1);
    if (world.state == World::Roaming && gopause) world.state = World::Paused;
    else if (world.state == World::Paused && !gopause) world.state = World::Roaming;
    return 0;
}

int Game::loadWorldFile(lua_State *ctx)
{
    bool res = WorldLoader::loadWorldFile(lua_tostring(ctx, 1), world);
    lua_pushboolean(ctx, res);
    return 1;
}

int Game::loadEventFile(lua_State *ctx)
{
    bool res = WorldLoader::loadDialogueNodes(lua_tostring(ctx, 1));
    lua_pushboolean(ctx, res);
    return 1;
}

int Game::movePlayer(lua_State *ctx)
{
    world.movePlayer(lua_tonumber(ctx, 1), lua_tonumber(ctx, 2));
    return 0; // TODO: return boolean whether move was successful or not
}

void Game::initialize(lua_State *ctx)
{
    const struct luaL_Reg world_funcs [] = {
        {"setPaused", pauseWorld},
        {"loadWorld", loadWorldFile},
        {"loadEvents", loadEventFile},
        {"movePlayer", movePlayer},
         {NULL, NULL}  /* sentinel */
       };
    luaL_openlib(ctx, "World", world_funcs, 0);
}

void Game::update(lua_State *ctx, float dt)
{
    switch(world.state) {
    case World::Roaming:
        lua_getglobal(ctx, "update");
        lua_pushnumber(ctx, double(dt));
        lua_pcall(ctx, 1, 0, 0);
        world.update(dt);
        break;
    case World::Transitioning:
//        world.updateTransition(dt);
        break;
    case World::Paused:
        lua_getglobal(ctx, "update_paused");
        lua_pushnumber(ctx, double(dt));
        lua_pcall(ctx, 1, 0, 0);
        break;
    case World::Dialogue:
//        world.updateDialogue(dt);
        break;
    }
}

void Game::draw(lua_State *ctx, Window &window)
{
    world.draw(window);

    lua_getglobal(ctx, "draw");
    lua_pcall(ctx, 0, 0, 0);
}
