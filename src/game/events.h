#ifndef EVENTS_H
#define EVENTS_H

#include <unordered_map>
#include <string>

struct Event;

struct EventNode {
    int visited = 0;
    std::vector<Event> events;

    bool empty() const { return events.size() == 0; }
};

struct Event
{
    enum Type {
        Empty,
        Dialogue,
    } type;

    Event(Type t) : type(t) {};

    std::string text;

    static EventNode visitNode(std::string id);

    static void registerNode(std::string id, EventNode node);

private:
    static std::unordered_map<std::string, EventNode> event_register;
};

#endif // EVENTS_H
