#include "system.h"

#include <iostream>

namespace {
    struct FuncBinding {
        lua_CFunction callback;
        const char * name;
    };

//    FuncBinding global_funcs[] = {
//    };
//    constexpr size_t global_func_count = std::size(global_funcs);

    struct WorldState {
        bool paused = false;
    } worldstate;

    int pauseWorld(lua_State *ctx) {
        worldstate.paused = lua_toboolean(ctx, 1);
        return 0;
    }

    FuncBinding world_funcs[] = {
        {pauseWorld, "setPaused"}
    };
}

System::State System::s_state;

int System::quit(lua_State *ctx)
{
    s_state.running = false;
    return 0;
}

int System::getActionHeld(lua_State *ctx)
{
    std::string action = lua_tostring(ctx, 1);
    lua_pushboolean(ctx, s_state.controls.get(action).held());
    return 1;
}

int System::getActionPressed(lua_State *ctx)
{
    std::string action = lua_tostring(ctx, 1);
    lua_pushboolean(ctx, s_state.controls.get(action).pressed());
    return 1;
}

int System::setPen(lua_State *ctx)
{
    s_state.window.setPen(lua_tonumber(ctx, 1), lua_tonumber(ctx, 2), lua_tonumber(ctx, 3), lua_tonumber(ctx, 4));
    return 0;
}

int System::drawRect(lua_State *ctx)
{
    s_state.window.rect(lua_tonumber(ctx, 1), lua_tonumber(ctx, 2), lua_tonumber(ctx, 3), lua_tonumber(ctx, 4));
    return 0;
}

int System::drawSprite(lua_State *ctx)
{
    s_state.window.draw(lua_tostring(ctx, 1), lua_tonumber(ctx, 2), lua_tonumber(ctx, 3));
    return 0;
}

int System::playSFX(lua_State *ctx)
{
    Audio::playSFX(lua_tostring(ctx, 1));
    return 0;
}

int System::playMusic(lua_State *ctx)
{
    Audio::playMusic(lua_tostring(ctx, 1), lua_tonumber(ctx, 2));
    return 0;
}

int System::stopMusic(lua_State *ctx)
{
    Audio::playMusic("", lua_tonumber(ctx, 1));
    return 0;
}

bool System::initLua()
{
    m_lua_context = luaL_newstate();
    luaL_openlibs(m_lua_context);

    // configure global engine calls
//    for(size_t i = 0; i < global_func_count; ++i) {
//        lua_pushcfunction(m_lua_context, global_funcs[i].callback);
//        lua_setglobal(m_lua_context, global_funcs[i].name);
//    }

    // configure "World" calls
    lua_newtable(m_lua_context);
    for(size_t i = 0; i < std::size(world_funcs); ++i) {
        lua_pushstring(m_lua_context, world_funcs[i].name);
        lua_pushcfunction(m_lua_context, world_funcs[i].callback);
        lua_rawset(m_lua_context, -3);
    }
    lua_setglobal(m_lua_context, "World");

    // configure "System" calls
    FuncBinding sysfuncs[] = {
        {quit, "quit"},
        {getActionHeld, "getActionHeld"},
        {getActionPressed, "getActionPressed"},
        {setPen, "setPen"},
        {drawRect, "drawRect"},
        {drawSprite, "drawSprite"},
        {playSFX, "playSFX"},
        {playMusic, "playMusic"},
        {stopMusic, "stopMusic"}
    };

    lua_newtable(m_lua_context);
    for(size_t i = 0; i < std::size(sysfuncs); ++i) {
        lua_pushstring(m_lua_context, sysfuncs[i].name);
        lua_pushcfunction(m_lua_context, sysfuncs[i].callback);
        lua_rawset(m_lua_context, -3);
    }
    lua_setglobal(m_lua_context, "System");

    int status = luaL_loadfile(m_lua_context, "resources/scripts/main.lua");
    if (status) {
        std::cerr << "ERROR: Couldn't load main.lua: " << lua_tostring(m_lua_context, -1);
        return false;
    }
    lua_pcall(m_lua_context, 0, 0, 0);

    lua_getglobal(m_lua_context, "init"); // find the function
    lua_pcall(m_lua_context, 0, 0, 0);
    return true;
}

bool System::initRendering()
{
    return s_state.window.initialize();
}

bool System::initAudio()
{
    return Audio::init();
}

bool System::initialize()
{
    // initLua() needs to act last for any of its system calls to function
    // yucky
    if (initRendering() && initAudio() && initLua()) {
        s_state.running = true;
    }
    return s_state.running;
}

void System::release()
{
    lua_close(m_lua_context);
}

void System::update(float dt)
{
    glfwPollEvents();
    s_state.controls.update(s_state.window);

    if (worldstate.paused) {
        lua_getglobal(m_lua_context, "update_paused");
        lua_pushnumber(m_lua_context, double(dt));
        lua_pcall(m_lua_context, 1, 0, 0);
    } else {
        lua_getglobal(m_lua_context, "update");
        lua_pushnumber(m_lua_context, double(dt));
        lua_pcall(m_lua_context, 1, 0, 0);
    }

    if (glfwWindowShouldClose(s_state.window)) {
        s_state.running = false;
    }

    // flush any lua print functions
    std::cout << std::flush;
    std::cerr << std::flush;
    std::clog << std::flush;
}

void System::renderScreen()
{
    s_state.window.renderWindow();

    // lua render callbacks; it is up to the client to trigger gameworld draws incase we are in a UI heavy scene
    lua_getglobal(m_lua_context, "draw");
    lua_pcall(m_lua_context, 0, 0, 0);
}


bool System::running() const
{
    return s_state.running;
}
