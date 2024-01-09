#ifndef RESOURCELOCATOR_H
#define RESOURCELOCATOR_H

#include <string>

class ResourceLocator
{
    static std::string root_path;

public:
    static void setRootPath(std::string path);

    static std::string getPackagePath();

    static std::string getPathPNG(std::string name);
    static std::string getPathScript(std::string name);
    static std::string getPathEvents(std::string name);
    static std::string getPathMap(std::string name);
    static std::string getPathWAV(std::string name);
};

#endif // RESOURCELOCATOR_H
