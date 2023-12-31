#include "worldcell.h"



int WorldCell::getTileAt(float pos_x, float pos_y) const
{
    int cx = std::floor(pos_x/float(tile_size_px));
    int cy = std::floor(pos_y/float(tile_size_px));
    size_t idx = cx + cy * cell_width_tiles;
    for(size_t k = layers.size() - 1; k >= 0; --k) {
        if (layers[k][idx] != 0) {
            return layers[k][idx];
        }
    }
    return 0;
}

void WorldCell::forEachTile(bool getScreenLocs, std::function<void (int, int, int)> callback) const
{
    int scaler = getScreenLocs ? tile_size_px : 1;
    for(int i = 0; i < cell_width_tiles; ++i) {
        for(int j = 0; j < cell_height_tiles; ++j) {
            size_t idx = i + j * cell_width_tiles;
            int tile = 0;
            for(size_t k = layers.size() - 1; k >= 0; --k) {
                if (layers[k][idx] != 0) {
                    tile = layers[k][idx];
                    // for now, we'll use a reverse-painter's algorithm and stop if we find a non-zero tile
                    break;
                }
            }
            if (tile != 0) callback(i * scaler, j * scaler, tile);
        }
    }
}

void WorldCell::forEachEntity(LevelEntity::Type type, std::function<void (const LevelEntity &)> callback) const
{
    for(const LevelEntity &e : level_entities) {
        if (e.type == type) {
            callback(e);
        }
    }
}
