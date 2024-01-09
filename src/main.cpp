#include <stdio.h>

#include <lua.hpp>

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <chrono>

#include "core/system.h"
#include "core/resourcelocator.h"

static constexpr int target_fps = 60;
static constexpr float timestep = 1.0f / float(target_fps);
static constexpr float physics_timestep = timestep * 0.43f; // just pick something lower that's not resonant

typedef std::chrono::steady_clock::time_point time_pt;

time_pt time() {
    return std::chrono::high_resolution_clock::now();
}

float delta_time(const time_pt &t1, const time_pt &t2) {
    return std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1).count();
}

int main(int argc, char **argv)
{
    System system;

    if (argc > 1) ResourceLocator::setRootPath(argv[1]);

    if (!system.initialize()) {
        std::cerr << "ERROR: System initialization failed." << std::endl;
        return -1;
    }

    auto gameTime = time();
    auto lastGameTime = gameTime;

    float accumulator = 0.0f;

    while (system.running()) {
        gameTime = time();
        float deltaTime = delta_time(lastGameTime, gameTime);
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }
        accumulator += deltaTime;
        lastGameTime = gameTime;

        while (accumulator >= physics_timestep) {
            system.update(physics_timestep);
            accumulator -= physics_timestep;
        }

        system.renderScreen();
    }

    system.release();

    return 0;
}
