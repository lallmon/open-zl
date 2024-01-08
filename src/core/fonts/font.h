#ifndef FONT_H
#define FONT_H

#include <cstdint>

enum FontName {
    MagoFont3,

    FontCount,
};

extern int magofont3_width;
extern int magofont3_height;
extern uint8_t magofont3_data[];
extern uint8_t magofont3_glyph_widths[];

class Font
{
    int width = 0, height = 0;
    unsigned int texture_id = 0;
    uint8_t * glyph_widths;

public:
    void bind() const;
    void getGlyphUVs(int idx, float * uvs) const;
    int getGlyphWidth(int idx) const;

    int getGlyphWidth() const;
    int getGlyphHeight() const;

    static const Font load(FontName font);

};

#endif // FONT_H
