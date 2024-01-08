#include "world.h"

bool World::startTransition(int cx, int cy, float px, float py, bool teleport)
{
    if (!isValidLoc(cx, cy, px, py)) {
        std::cerr << "ERROR: Could not transition to cell " << cx << " " << cy << " at point " << px << " " << py << std::endl;
        return false;
    }
    transition.target_cell[0] = cx;
    transition.target_cell[1] = cy;
    transition.target_pos[0] = px;
    transition.target_pos[1] = py;
    transition.is_teleport = teleport;
    transition.timer = transition.timer_reset;
    state = Transitioning;
    return true;
}

//bool World::isValidLoc(float x, float y)
//{
//    int cx = std::floor(x/float(cell_width_px));
//    int cy = std::floor(y/float(cell_height_px));

//    if (world_cells.find(cx) == world_cells.end()) {
//        return false;
//    }
//    if (world_cells.at(cx).find(cy) == world_cells.at(cx).end()) {
//        return false;
//    }
//    const WorldCell &c = world_cells.at(cx).at(cy);

//    int px = x - cx * cell_width_px;
//    int py = y - cy * cell_height_px;

//    return c.getTileAt(px, py) != 8;
//}

bool World::isValidLoc(int cx, int cy, float x, float y) const
{
    if (world_cells.find(cx) == world_cells.end()) return false;
    if (world_cells.at(cx).find(cy) == world_cells.at(cx).end()) return false;

    const WorldCell &c = world_cells.at(cx).at(cy);
    return c.getTileAt(x, y) != 8;
}

bool World::canMoveToCell(int dcell_x, int dcell_y, float x, float y) const
{
    int cx = player.world_cell_x + dcell_x;
    int cy = player.world_cell_y + dcell_y;

    return isValidLoc(cx, cy, x, y);
}

void World::checkPlayerTransitions()
{
    const WorldCell &active_cell = world_cells.at(player.world_cell_x).at(player.world_cell_y);
    bool starting_teleport = false;
    float x, y;
    active_cell.forEachEntity(LevelEntity::Exit, [&](const LevelEntity &exit) {
        if (starting_teleport) return;
        if (exit.x < player.x && exit.y < player.y && player.x < (exit.x + exit.width) && player.y < (exit.y + exit.height)) {
            starting_teleport = teleportPlayer(exit.data.exit.to_world_x, exit.data.exit.to_world_y);
        }
    });

    if (starting_teleport) return;

    if (player.x < 0) {
        if (!startTransition(player.world_cell_x-1, player.world_cell_y, player.x + cell_width_px, player.y)) {
            player.x = 0;
        }
    } else if (player.x > cell_width_px) {
        if (!startTransition(player.world_cell_x+1, player.world_cell_y, player.x - cell_width_px, player.y)) {
            player.x = cell_width_px;
        }
    }

    if (player.y < 0) {
        if (!startTransition(player.world_cell_x, player.world_cell_y-1, player.x, player.y + cell_height_px)) {
            player.y = 0;
        }
    } else if (player.y > cell_height_px) {
        if (!startTransition(player.world_cell_x, player.world_cell_y+1, player.x, player.y - cell_height_px)) {
            player.y = cell_height_px;
        }
    }
}

bool World::teleportPlayer(float x, float y)
{
    int cx = std::floor(x/float(cell_width_px));
    int cy = std::floor(y/float(cell_height_px));
    float px = x - cx * cell_width_px;
    float py = y - cy * cell_height_px;

    return startTransition(cx, cy, px, py, true);
}

void World::drawCell(const WorldCell &cell, Window &window, float offset_x, float offset_y) const
{
    float dx = float(tile_size_px) / 128.0f;
    float dy = float(tile_size_px) / 128.0f;
    int hcount = 128/tile_size_px;

    window.bindTexture("tileset", tile_size_px, tile_size_px);
    cell.forEachTile(true, [&](int x, int y, int t) {
        float ux = dx * float(t % hcount);
        float uy = dy * float(t / hcount);
        window.setClip(ux, uy, ux + dx, uy + dy);
        window.drawActive(x + offset_x, y + offset_y);
    });
    window.setClip(0, 0, 1, 1);
}

void World::reset()
{
    player.world_cell_x = m_player_start_x;
    player.world_cell_y = m_player_start_y;

    WorldCell &active_cell = world_cells.at(m_player_start_x).at(m_player_start_y);
    player.x = active_cell.spawn_x;
    player.y = active_cell.spawn_y;

    state = Roaming;
}

void World::update(float dt)
{
    checkPlayerTransitions();
}

void World::updateTransition(float dt)
{
    transition.timer -= dt;
    if (transition.timer < 0) {
        player.world_cell_x = transition.target_cell[0];
        player.world_cell_y = transition.target_cell[1];
        player.x = transition.target_pos[0];
        player.y = transition.target_pos[1];
        state = Roaming;
    }
    if (transition.is_teleport && transition.timer < transition.timer_reset/2) {
        player.world_cell_x = transition.target_cell[0];
        player.world_cell_y = transition.target_cell[1];
        player.x = transition.target_pos[0];
        player.y = transition.target_pos[1];
    }
}

void World::updateDialogue(float dt)
{

}

void World::draw(Window &window) const
{
    const WorldCell &active_cell = world_cells.at(player.world_cell_x).at(player.world_cell_y);

    float offsetx = 0;
    float offsety = 0;

    float player_draw_loc_x = player.x;
    float player_draw_loc_y = player.y;

    if (state == Transitioning && !transition.is_teleport) {
        float f = transition.timer/transition.timer_reset;
        if (f < 0) f = 0;

        // TODO: move this somewhere, e.g. window.getVirtualSize()
        constexpr float width = 256;
        constexpr float height = 144;

        float delta[2] = {
            float(transition.target_cell[0] - player.world_cell_x),
            float(transition.target_cell[1] - player.world_cell_y)
        };

        // incoming cell moves from 1 to 0 in the direction of the transition
        offsetx = width * f * delta[0];
        offsety = height * f * delta[1];

        const WorldCell &next_cell = world_cells.at(transition.target_cell[0]).at(transition.target_cell[1]);
        drawCell(next_cell, window, offsetx, offsety);

        // lerp player draw location to new position
        player_draw_loc_x = player.x * f + transition.target_pos[0] * (1-f);
        player_draw_loc_y = player.y * f + transition.target_pos[1] * (1-f);

        // active cell moves from 0 to -1 in the direction of the transition
        f -= 1;
        offsetx = width * f * delta[0];
        offsety = height * f * delta[1];
    }

    drawCell(active_cell, window, offsetx, offsety);

    active_cell.forEachEntity(LevelEntity::Texture, [&](const LevelEntity &tex) {
        window.draw(tex.data.texture.name, tex.x, tex.y, tex.width, tex.height);
    });

    window.setPen(1, 1, 1, 1);
    window.rect(player_draw_loc_x - 6, player_draw_loc_y - 6, 12, 12);

    if (state == Transitioning && transition.is_teleport) {
        float f = (transition.timer_reset/2 - transition.timer) / (transition.timer_reset/2);
        f *= f;
        if (f > 1) f = 1;
        f = 1 - f;

        window.setPen(0, 0, 0, f);
        window.rect(0, 0, 256, 144);
        window.setPen(1, 1, 1, 1);
    }
}

void World::movePlayer(float dx, float dy)
{
    // TODO: add collision checking;
    player.x += dx;
    player.y += dy;
}
