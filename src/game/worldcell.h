#ifndef WORLDCELL_H
#define WORLDCELL_H

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <functional>

// these constants subject to change; closely linked to resolution
constexpr int cell_width_px = 256;
constexpr int cell_height_px = 144;
constexpr int tile_size_px = 16;

// derived constants
constexpr int cell_width_tiles = cell_width_px / tile_size_px;
constexpr int cell_height_tiles = cell_height_px / tile_size_px;
constexpr int tiles_per_cell = cell_width_px * cell_height_px / tile_size_px / tile_size_px;

typedef std::array<int, tiles_per_cell> TileLayer;

struct LevelEntity {
    int x, y, width, height;

    enum Type {
        None,
        Exit,
        Texture
    } type = None;

    struct ExitData { int to_world_x, to_world_y; };
    struct TextureData { char name[40]; };

    union Data {
        ExitData exit;
        TextureData texture;
    } data;

    void setAsExit(int x, int y) {
        type = Exit;
        data.exit.to_world_x = x;
        data.exit.to_world_y = y;
    }

    void setAsTexture(std::string name) {
        if (name.length() > 39) {
            std::clog << "WARNING: Could not create Texture LevelEntity; texture named " << name << " has more than 40 characters.";
            return;
        }
        type = Texture;
        std::strcpy(data.texture.name, name.c_str());
    }
};

struct WorldCell
{
    int cell_horizontal_index, cell_vertical_index;

    std::vector<TileLayer> layers;
    std::vector<LevelEntity> level_entities;

    bool has_player_spawn = false;
    int spawn_x, spawn_y;

    int getTileAt(float pos_x, float pos_y) const;

    void forEachTile(bool getScreenLocs, std::function<void(int x, int y, int tile)> callback) const;

    void forEachEntity(LevelEntity::Type type, std::function<void(const LevelEntity&)> callback) const;
};

#endif // WORLDCELL_H
