#include "resourcelocator.h"

#include <sys/stat.h>
#include <iostream>

std::string ResourceLocator::root_path = "./resources/";

void ResourceLocator::setRootPath(std::string path)
{
     struct stat sb;

     if (stat(path.c_str(), &sb) == 0) {
         root_path = path + "/";
     } else {
         std::cerr << "ERROR: requested resource path " << path << " is not valid.";
     }
}

std::string ResourceLocator::getPackagePath()
{
    return root_path + "scripts/?.lua";
}

std::string ResourceLocator::getPathPNG(std::string name)
{
    return root_path + "textures/" + name + ".png";
}

std::string ResourceLocator::getPathScript(std::string name)
{
    return root_path + "scripts/" + name + ".lua";
}

std::string ResourceLocator::getPathEvents(std::string name)
{
    return root_path + "dialogue/" + name + ".twee";
}

std::string ResourceLocator::getPathMap(std::string name)
{
    return root_path + "levels/" + name + ".ldtk";
}

std::string ResourceLocator::getPathWAV(std::string name)
{
    return root_path + "audio/" + name + ".wav";
}
