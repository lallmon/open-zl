#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <glfw3.h>

#include <unordered_map>
#include <string>

class Window
{
    GLFWwindow * m_handle;

    unsigned int m_texture_shader;
    unsigned int m_texture_vao;

    unsigned int m_pen_texture;
    float m_pen_set[4] = { 1, 1, 1, 1 };

    struct Texture {
        unsigned int id = 0;
        bool failed = false;
        int w, h;
    };
    std::unordered_map<std::string, Texture> m_texture_map;

    bool tryLoadTexture(std::string tex);

public:
    bool initialize();

    void renderWindow();

    void draw(std::string texture_name, float x, float y);

    void setPen(float r, float g, float b, float a);
    void rect(float x, float y, float w, float h);

    operator GLFWwindow *() { return m_handle; }

};

#endif // WINDOW_H
