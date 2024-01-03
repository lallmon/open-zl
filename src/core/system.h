#ifndef SYSTEM_H
#define SYSTEM_H

#include <lua.hpp>

#include "core/window.h"
#include "core/audio.h"
#include "core/controls.h"

#include "game/worldloader.h"

class System
{
    // todo: move this into its own module. need to figure out how to cleanly handle system level bindings with
    lua_State *m_lua_context = nullptr;

    struct State {
        Controls controls;
        Window window;
        Audio audio;
        World world;
        bool running = false;
    } static s_state;

    // System Lua callbacks
    static int quit(lua_State*);

    // Controls Lua callbacks
    static int getActionHeld(lua_State*);
    static int getActionPressed(lua_State*);

    // Rendering Lua callbacks
    static int setPen(lua_State*);
    static int drawRect(lua_State*);
    static int drawSprite(lua_State*);

    // Audio Lua callbacks
    static int playSFX(lua_State*);
    static int playMusic(lua_State*);
    static int stopMusic(lua_State*);

    // Gameplay Lua callbacks
    static int loadWorldFile(lua_State*);
    static int loadEventFile(lua_State*);
    static int movePlayer(lua_State*);

    // System configuration
    bool initLua();
    bool initRendering();
    bool initAudio();
public:
    bool initialize();
    void release();

    void update(float dt);
    void renderScreen();

    bool running() const;
};

#endif // SYSTEM_H
