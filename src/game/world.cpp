#include "world.h"

bool World::isValidLoc(float x, float y)
{
    int cx = std::floor(x/float(cell_width_px));
    int cy = std::floor(y/float(cell_height_px));

    if (world_cells.find(cx) == world_cells.end()) {
        return false;
    }
    if (world_cells.at(cx).find(cy) == world_cells.at(cx).end()) {
        return false;
    }
    const WorldCell &c = world_cells.at(cx).at(cy);

    int px = x - cx * cell_width_px;
    int py = y - cy * cell_height_px;

    return c.getTileAt(px, py) != 8;
}

bool World::canMoveToCell(int dcell_x, int dcell_y, float x, float y) const
{
    int cx = player.world_cell_x + dcell_x;
    int cy = player.world_cell_y + dcell_y;
    if (world_cells.find(cx) == world_cells.end()) {
        return false;
    }
    if (world_cells.at(cx).find(cy) == world_cells.at(cx).end()) {
        return false;
    }
    const WorldCell &next = world_cells.at(cx).at(cy);

    return next.getTileAt(x, y) != 8;
}

void World::checkPlayerTransitions()
{
    const WorldCell &active_cell = world_cells.at(player.world_cell_x).at(player.world_cell_y);
    bool teleport = false;
    float x, y;
    active_cell.forEachEntity(LevelEntity::Exit, [&](const LevelEntity &exit) {
        if (exit.x < player.x && exit.y < player.y && player.x < (exit.x + exit.width) && player.y < (exit.y + exit.height)) {
            teleport = true;
            x = exit.data.exit.to_world_x;
            y = exit.data.exit.to_world_y;
        }
    });

    if (teleport && teleportPlayer(x, y)) return;

    // first check boundaries
    if (player.x < 0) {
        if (canMoveToCell(-1, 0, player.x + cell_width_px, player.y)) {
            player.world_cell_x--;
            player.x += cell_width_px;
        } else {
            player.x = 0;
        }
    } else if (player.x > cell_width_px) {
        if (canMoveToCell(1, 0, player.x - cell_width_px, player.y)) {
            player.world_cell_x++;
            player.x -= cell_width_px;
        } else {
            player.x = cell_width_px;
        }
    }

    if (player.y < 0) {
        if (canMoveToCell(0, -1, player.x, player.y + cell_height_px)) {
            player.y += cell_height_px;
            player.world_cell_y--;
        } else {
            player.y = 0;
        }
    } else if (player.y > cell_height_px) {
        if (canMoveToCell(0, 1, player.x, player.y - cell_height_px)) {
            player.y -= cell_height_px;
            player.world_cell_y++;
        } else {
            player.y = cell_height_px;
        }
    }
}

bool World::teleportPlayer(float x, float y)
{
    if (!isValidLoc(x, y)) {
        std::clog << "WARNING: Unable to teleport player to requested location, " << x << " " << y << std::endl;
        return false;
    }

    player.world_cell_x = std::floor(x/float(cell_width_px));
    player.world_cell_y = std::floor(y/float(cell_height_px));
    player.x = x - player.world_cell_x * cell_width_px;
    player.y = y - player.world_cell_y * cell_height_px;
    return true;
}

void World::reset()
{
    player.world_cell_x = m_player_start_x;
    player.world_cell_y = m_player_start_y;

    WorldCell &active_cell = world_cells.at(m_player_start_x).at(m_player_start_y);
    player.x = active_cell.spawn_x;
    player.y = active_cell.spawn_y;

    state = FreeRoam;
}

void World::update(float dt)
{
    checkPlayerTransitions();
}

void World::draw(Window &window) const
{
    const WorldCell &active_cell = world_cells.at(player.world_cell_x).at(player.world_cell_y);

    float dx = float(tile_size_px) / 128.0f;
    float dy = float(tile_size_px) / 128.0f;
    int hcount = 128/tile_size_px;

    window.bindTexture("tileset", tile_size_px, tile_size_px);
    active_cell.forEachTile(true, [&](int x, int y, int t) {
        float ux = dx * float(t % hcount);
        float uy = dy * float(t / hcount);
        window.setClip(ux, uy, ux + dx, uy + dy);
        window.drawActive(x, y);
    });
    window.setClip(0, 0, 1, 1);

    active_cell.forEachEntity(LevelEntity::Texture, [&](const LevelEntity &tex) {
        window.draw(tex.data.texture.name, tex.x, tex.y, tex.width, tex.height);
    });

    window.setPen(1, 1, 1, 1);
    window.rect(player.x - 6, player.y - 6, 12, 12);
}

void World::movePlayer(float dx, float dy)
{
    // TODO: add collision checking;
    player.x += dx;
    player.y += dy;
}
