#ifndef WORLD_H
#define WORLD_H

#include <unordered_map>

#include "core/window.h"

#include "game/worldcell.h"
#include "game/playerdata.h"
#include "game/dialogueviewer.h"

class WorldLoader;
class Game;

class World
{
    enum WorldState {
        Roaming,
        Paused,
        Transitioning,
        Dialogue
    };

    int m_player_start_x = 0, m_player_start_y = 0;
    std::unordered_map<int, std::unordered_map<int, WorldCell>> world_cells;

    PlayerData player;
    WorldState state;
    DialogueSequence active_dialogue;

    struct TransitionData {
        float timer = 0.0f;
        static constexpr float timer_reset = 0.8f;
        bool is_teleport = false;
        int target_cell[2];
        float target_pos[2];
    } transition;

    bool startTransition(int cx, int cy, float px, float py, bool teleport = false);

    bool isValidLoc(int cx, int cy, float x, float y) const; // todo: this should take a collision mask argument

    bool canMoveToCell(int delta_cell_x, int delta_cell_y, float at_x, float at_y) const;
    void checkPlayerTransitions();
    bool teleportPlayer(float x, float y);

    void drawCell(const WorldCell &cell, Window &window, float offset_x = 0, float offset_y = 0) const;

    friend WorldLoader;
    friend Game;

    void setDialogue(DialogueSequence seq);
public:
    void reset();
    void update(float dt);
    void updateTransition(float dt);
    void updateDialogue(float dt);
    void draw(Window &window) const;

    void movePlayer(float dx, float dy);

};

#endif // WORLD_H
