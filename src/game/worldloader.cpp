#include "worldloader.h"

#include <iostream>
#include <fstream>
#include <unordered_map>

#include "json.hpp"

#include "core/resourcelocator.h"

#include "game/worldcell.h"


namespace {

    static bool hasFieldInstances(const nlohmann::json &node, size_t count) {
        if (node["fieldInstances"].is_null()) return false;
        if (node["fieldInstances"].size() < count) return false;
        for(size_t i = 0; i < count; ++i) {
            if (node["fieldInstances"][i]["__value"].is_null()) return false;
        }
        return true;
    }

    static bool tryLoadLevel(const nlohmann::json &level, WorldCell &cell) {
        int px = level["worldX"].get<int>();
        int py = level["worldY"].get<int>();
        if ((px % int(cell_width_px)) != 0 || (py % int(cell_height_px)) != 0) {
            std::clog << "WARNING: Level " << level["identifier"] << " was not loaded; it is not at an integer cell boundary.";
            return false;
        }
        cell.cell_horizontal_index = int(std::floor(float(px) / cell_width_px));
        cell.cell_vertical_index = int(std::floor(float(py) / cell_height_px));

        for(const auto &layer : level["layerInstances"]) {
            if (layer["__type"] == "Tiles") {
                TileLayer l;
                l.fill(0);

                for(const auto &tile : layer["gridTiles"]) {
                    int tx = tile["px"][0].get<int>() / tile_size_px;
                    int ty = tile["px"][1].get<int>() / tile_size_px;

                    if (tx < 0 || ty < 0 || tx >= cell_width_tiles || ty >= cell_height_tiles) continue;

                    size_t idx = size_t(tx) + size_t(ty * cell_width_px/tile_size_px);
                    l[idx] = tile["t"].get<int>();
                }

                cell.layers.push_back(l);
            } else if (layer["__type"] == "Entities") {
                for(const auto &e: layer["entityInstances"]) {
                    LevelEntity entity;
                    std::string type = e["__identifier"];
                    if (type == "Exit") {
                        if (!hasFieldInstances(e, 2)) break;
                        entity.setAsExit(e["fieldInstances"][0]["__value"].get<int>(), e["fieldInstances"][1]["__value"].get<int>());
                    } else if (type == "Texture") {
                        if (!hasFieldInstances(e, 1)) break;
                        entity.setAsTexture(e["fieldInstances"][0]["__value"].get<std::string>());
                    } else if (type == "PlayerSpawn") {
                        cell.has_player_spawn = true;
                        cell.spawn_x = int((e["__grid"][0].get<float>() + 0.5f) * float(tile_size_px));
                        cell.spawn_y =int((e["__grid"][1].get<float>() + 0.5f) * float(tile_size_px));
                    }
                    if (entity.type != LevelEntity::None) {
                        entity.x = e["__grid"][0].get<int>() * tile_size_px;
                        entity.y = e["__grid"][1].get<int>() * tile_size_px;
                        entity.width = e["width"].get<int>();
                        entity.height = e["height"].get<int>();

                        cell.level_entities.push_back(entity);
                    }
                }
            }
        }

        return true;
    }


    void registerNode(std::string id, EventNode node) {
        if (!node.empty() && id.size() > 0) {
            if (id != "StoryTitle" && id != "StoryData") {
//                std::cout << "INFO: registering " << id << " with event count " << node.events.size() << std::endl;
                Event::registerNode(id, node);
            }
        }
    }
}

bool WorldLoader::loadWorldFile(std::string fname, World &world_data)
{
    fname = ResourceLocator::getPathMap(fname);

    world_data = World();

    std::ifstream file(fname);
    if (!file.good()) {
        std::cerr << "ERROR: World file at " << fname.c_str() << " does not exist" << std::endl;
        std::cerr << "       " << strerror(errno) << std::endl;
        return false;
    }

    nlohmann::json world;
    file >> world;

    for(auto &level : world["levels"]) {
        WorldCell cell;
        if (tryLoadLevel(level, cell)) {
            world_data.world_cells[cell.cell_horizontal_index][cell.cell_vertical_index] = cell;
            if (cell.has_player_spawn) {
                world_data.m_player_start_x = cell.cell_horizontal_index;
                world_data.m_player_start_y = cell.cell_vertical_index;
            }
        }
    }

    world_data.reset();

    return true;
}

bool WorldLoader::loadDialogueNodes(std::string fname)
{
    fname = ResourceLocator::getPathEvents(fname);

    std::ifstream file(fname);
    if (!file.good()) {
        std::cerr << "ERROR: Could not load " << fname.c_str() << std::endl;
        std::cerr << "       " << strerror(errno) << std::endl;
        return false;
    }
    EventNode node;
    std::string id;
    std::string line;
    while(getline(file, line)) {
        if (line.find(":: ") == 0) {
            registerNode(id, node);
            id = line.substr(3, line.length() - 3);
            size_t del = id.find(" {");
            if (del != std::string::npos) {
                id = id.substr(0, del);
            }
            node = EventNode();
        } else if (line.length() > 0 && line.find("//") > 0) {
            Event e(Event::Dialogue);
            e.text = line;
            node.events.push_back(e);
        }
    }
    registerNode(id, node);
    return true;
}
