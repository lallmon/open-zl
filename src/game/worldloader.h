#ifndef WORLDLOADER_H
#define WORLDLOADER_H

#include <string>

#include "game/world.h"
#include "game/events.h"

class WorldLoader
{

public:

    static bool loadWorldFile(std::string fname, World &world);

    static bool loadDialogueNodes(std::string fname);
};

#endif // WORLDLOADER_H
