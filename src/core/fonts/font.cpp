#include "font.h"

#include <string>
#include <fstream>

#include "glad/glad.h"
//#include "stb_image.h"

namespace {

//    void dumpFontData(std::string from_png, std::string to_file) {
//        from_png = "../../content/" + from_png + ".png";
//        std::string fname = "../../src/core/fonts/" + to_file + ".cpp";

//        int w, h;
//        unsigned char * data = stbi_load(from_png.c_str(), &w, &h, 0, 4);

//        std::ofstream file(fname);

//        file << "#pragma once\n\n#include <cstdint>\n\n";
//        file << "int " << to_file << "_width = " << w << ";\nint " << to_file << "_height = " << h <<";\n\n";
//        file << "uint8_t " << to_file << "_data[" << w * h * 4 << "] = {\n\t";
//        for(int i = 0; i < w*h*4; ++i) {
//            file << int(data[i]) << ", ";

//            if (i % 10000 == 0) file.flush();
//        }
//        file << "\n};\n\n";

//        // TODO: PARSE GLYPH WIDTHS HERE

//        stbi_image_free(data);
//    }

    static bool font_loaded[FontCount] = {false};

    static Font fonts[FontCount];
}

void Font::bind() const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Font::getGlyphUVs(int idx, float *uvs) const
{
    // texture is assumed to be 256 glyphs organized as a 16x16 grid
    constexpr int glyphs_wide = 16;
    constexpr int glyphs_tall = 16;

    const float dx = 1.0f/float(glyphs_wide);
    const float dy = 1.0f/float(glyphs_tall);

    uvs[0] = float(idx % glyphs_wide) * dx;
    uvs[1] = uvs[0] + dx;

    uvs[2] = float(idx/glyphs_wide) * dy;
    uvs[3] = uvs[2] + dy;
}

int Font::getGlyphWidth(int idx) const
{
    return glyph_widths[idx];
}

int Font::getGlyphWidth() const
{
    return width / 16; // texture is assumed to be 256 glyphs organized as a 16x16 grid
}

int Font::getGlyphHeight() const
{
    return height / 16; // texture is assumed to be 256 glyphs organized as a 16x16 grid
}

const Font Font::load(FontName font)
{
//    dumpFontData("font", "magofont3");
//    return Font();

    if (font == FontCount) return Font();
    if (font_loaded[font]) return fonts[font];

    Font &f = fonts[font];
    uint8_t * data;
    switch(font) {
    case MagoFont3:
        f.width = magofont3_width;
        f.height = magofont3_height;
        f.glyph_widths = magofont3_glyph_widths;
        data = magofont3_data;
        break;
    }

    glGenTextures(1, &f.texture_id);
    glBindTexture(GL_TEXTURE_2D, f.texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, f.width, f.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


    font_loaded[font] = true;

    return f;
}

float Font::getLineWidth(FontName font, std::string line, float scale)
{
    Font f = load(font);
    float len = -scale; //offset the first space
    for(const char &ch : line) {
        len += float(f.getGlyphWidth(ch) + 1) * scale;
    }
    return len;
}
