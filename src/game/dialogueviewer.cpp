#include "dialogueviewer.h"

#include <sstream>

#include "core/fonts/font.h"

DialogueViewer::DialogueViewer(std::string txt)
{
    std::stringstream ss(txt);

    std::string working_line;
    std::getline(ss, working_line, ' ');
    for(std::string line; std::getline(ss, line, ' ');) {
        float len = Font::getLineWidth(MagoFont3, working_line + " " + line, font_scale);
        if (len > max_line_width) {
            lines.push_back(working_line);
            character_count += working_line.size();
            working_line = line;
        } else {
            working_line += " " + line;
        }
    }
    if (working_line.size() > 0) {
        lines.push_back(working_line);
        character_count += working_line.size();
    }
}

void DialogueViewer::update(float dt)
{
    // TODO: add text scrolling
    character_position += dt * text_speed;
}

void DialogueViewer::draw(Window &window, float confirm_after_time) const
{
    // TODO: add text scrolling
    window.setPen(0, 0, 0, 1);
    window.rect(screen_gutter, 144 - screen_gutter - box_height, box_width, box_height);

    window.setPen(1, 1, 1, 1);
    int characters_printed = 0;
    float y = 144 - screen_gutter - box_height + box_gutter;
    for(const std::string &line : lines) {
       int max_chars = int(std::fmin(line.size(), character_position - characters_printed));
       window.print(line.substr(0, max_chars), screen_gutter + box_gutter, y, font_scale);
       characters_printed += max_chars;
       y += float(Font::load(MagoFont3).getGlyphHeight()) * font_scale + line_spacing;
    }

    if (ready(confirm_after_time) && std::fmod((character_position-float(character_count))/text_speed-confirm_after_time, 0.8f) < 0.6f) {
        window.setPen(1, 1, 1, 1);
        window.print(">", box_width + screen_gutter - screen_gutter, 144 - screen_gutter*2 - 2);
    }
}

bool DialogueViewer::ready(float for_time) const
{
    return (character_position - float(character_count))/text_speed > for_time;
}

std::queue<DialogueViewer> DialogueViewer::getNodes(std::vector<std::string> nodes)
{
    std::queue<DialogueViewer> d;
    for(std::string l : nodes) {
        d.push(l);
    }
    return d;
}
