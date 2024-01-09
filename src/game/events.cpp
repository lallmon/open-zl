#include "events.h"

#include <iostream>

std::unordered_map<std::string, EventNode> Event::event_register;

EventNode Event::visitNode(std::string id)
{
    if (event_register.find(id) != event_register.end()) {
        event_register.at(id).visited++;
        return event_register.at(id);
    }
    std::clog << "WARNING: request to visit unregistered node id: " << id << std::endl;
    return EventNode();
}

void Event::registerNode(std::string id, EventNode node)
{
    if (event_register.find(id) != event_register.end()) {
        std::clog << "WARNING: event node with id " << id << " registered multiple times; some data may be lost" << std::endl;
    }
    event_register.emplace(id, node);
}
