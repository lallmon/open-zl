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

int Game::startDialogue(lua_State *ctx)
{
    // TODO: Optionally allow calling this function with a string identifying a Dialogue node parsed from a TWEE file

    std::vector<std::string> lines;
    /* 1st argument must be a table (t) */
    luaL_checktype(ctx, 1, LUA_TTABLE);

    lua_pushnil(ctx);  /* first key */
    while (lua_next(ctx, 1) != 0) {
        if (lua_type(ctx, -1) == LUA_TSTRING) {
            const char * line = lua_tostring(ctx, -1);
            lines.push_back(line);
        }
        lua_pop(ctx, 1);
    }
    world.setDialogue(DialogueViewer::getNodes(lines));
    return 0;
}

void Game::initialize(lua_State *ctx)
{
    const struct luaL_Reg world_funcs [] = {
    {"setPaused", pauseWorld},
    {"loadWorld", loadWorldFile},
    {"loadEvents", loadEventFile},
    {"movePlayer", movePlayer},
    {"startDialogue", startDialogue},
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
        world.updateTransition(dt);
        break;
    case World::Paused:
        lua_getglobal(ctx, "update_paused");
        lua_pushnumber(ctx, double(dt));
        lua_pcall(ctx, 1, 0, 0);
        break;
    case World::Dialogue:
        world.updateDialogue(dt);
        break;
    }
}

void Game::draw(lua_State *ctx, Window &window)
{
    world.draw(window);

    lua_getglobal(ctx, "draw");
    lua_pcall(ctx, 0, 0, 0);
}
