#include "system.h"

#include <iostream>

#include "core/resourcelocator.h"

System::State System::s_state;

int System::quit(lua_State *ctx)
{
    s_state.running = false;
    return 0;
}

int System::getActionHeld(lua_State *ctx)
{
    std::string action = lua_tostring(ctx, 1);
    lua_pushboolean(ctx, getAction(action).held());
    return 1;
}

int System::getActionPressed(lua_State *ctx)
{
    std::string action = lua_tostring(ctx, 1);
    lua_pushboolean(ctx, getAction(action).pressed());
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
    int ct = lua_gettop(ctx);
    if (ct == 3) {
        s_state.window.draw(lua_tostring(ctx, 1), lua_tonumber(ctx, 2), lua_tonumber(ctx, 3));
    }
    return 0;
}

int System::print(lua_State *ctx)
{
    if (lua_gettop(ctx) < 3) {
        return 1;
    }
    std::string txt = lua_tostring(ctx, 1);
    int x = lua_tointeger(ctx, 2);
    int y = lua_tointeger(ctx, 3);
    float scale = 1.0f;
    if (lua_gettop(ctx) > 3) {
        scale = lua_tonumber(ctx, 4);
    }
    s_state.window.print(txt, x, y, scale);
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

    {
        std::string ppath = "package.path = package.path .. ';" + ResourceLocator::getPackagePath() + "'";
        luaL_dostring(m_lua_context, ppath.c_str());
    }

    // configure "World" calls
    s_state.game.initialize(m_lua_context);

    // configure "System" calls
    const struct luaL_Reg sys_funcs [] = {
        {"quit", quit},
        {"getActionHeld", getActionHeld},
        {"getActionPressed", getActionPressed},
        {"setPen", setPen},
        {"drawRect", drawRect},
        {"print", print},
        {"drawSprite", drawSprite},
        {"playSFX", playSFX},
        {"playMusic", playMusic},
        {"stopMusic", stopMusic},
        {NULL, NULL}
    };
    luaL_openlib(m_lua_context, "System", sys_funcs, 0);

    std::string mainfile = ResourceLocator::getPathScript("main");
    int status = luaL_loadfile(m_lua_context, mainfile.c_str());
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

    s_state.game.update(m_lua_context, dt);

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
    s_state.game.draw(m_lua_context, s_state.window);
    s_state.window.renderWindow();
}


bool System::running() const
{
    return s_state.running;
}

Button System::getAction(std::string action)
{
    return s_state.controls.get(action);
}
