#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <queue>

#include "core/window.h"

class DialogueViewer
{
    static constexpr int screen_gutter = 6;
    static constexpr int box_width = 256 - screen_gutter*2;
    static constexpr int box_height = 52;
    static constexpr int box_gutter = 8;
    static constexpr int line_spacing = 4; // NOT leading! this is the gap between descender line and next ascender line
    static constexpr int max_line_width = box_width - box_gutter * 2;

    static constexpr float font_scale = 1.0f;

    static constexpr float text_speed = 24.0f; // characters per second;

    std::vector<std::string> lines;
    float character_position = 0.0f;

    int character_count = 0;

    DialogueViewer(std::string line);

public:
    void update(float dt);

    void draw(Window &window, float confirm_after_time) const;

    // call ready(1.0f), for example, if wanting to wait until characters have all been shown for 1 second
    bool ready(float for_time) const;

    static std::queue<DialogueViewer> getNodes(std::vector<std::string> nodes);
};

typedef std::queue<DialogueViewer> DialogueSequence;

#endif // DIALOGUE_H
