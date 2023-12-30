#ifndef WORLDLOADER_H
#define WORLDLOADER_H

#include <string>

#include "game/world.h"

class WorldLoader
{

public:

    static bool loadWorldFile(std::string fname, World &world);
};

#endif // WORLDLOADER_H
