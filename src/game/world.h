#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>

#include "core/window.h"

#include "game/worldcell.h"
#include "game/playerdata.h"

class WorldLoader;

enum WorldState {
    FreeRoam,
    Transition,
};

class World
{
    int m_player_start_x = 0, m_player_start_y = 0;
    std::unordered_map<int, std::unordered_map<int, WorldCell>> world_cells;

    PlayerData player;
    WorldState state;

    bool isValidLoc(float x, float y); // todo: this should take a collision mask argument

    bool canMoveToCell(int dcell_x, int dcell_y, float x, float y) const;
    void checkPlayerTransitions();
    bool teleportPlayer(float x, float y);

    friend WorldLoader;
public:
    void reset();
    void update(float dt);
    void draw(Window &window) const;

    void movePlayer(float dx, float dy);

    WorldState getWorldState() const;
};

#endif // WORLD_H
