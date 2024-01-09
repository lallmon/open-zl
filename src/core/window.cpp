#include "window.h"

#include <iostream>
#include <algorithm>

#include "core/resourcelocator.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

constexpr int virtual_width = 256;
constexpr int virtual_height = 144;
constexpr float virtual_aspect_ratio = float(virtual_width) / float(virtual_height);

void framebufferResize(GLFWwindow * w, int width, int height)
{
    const float width_mult = float(width) / float(virtual_width);
    const float height_mult = float(height) / float(virtual_height);

    float frac_w, frac_h;
    const int mult = int(std::fmin(width_mult, height_mult));
    frac_w = float(mult * virtual_width) / float(width);
    frac_h = float(mult * virtual_height) / float(height);
    int left = float(width)*(1-frac_w)/2.0f;
    int bottom = float(height)*(1-frac_h)/2.0f;

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(left, bottom, float(width)*frac_w, float(height)*frac_h);

    // TODO: figure out how to orchestrate game redraws, e.g.:
    //    active->renderWindow();
    // so that the window dynamically redraws while resizing
    // (probably switching to render FBO is the best way / least muddy)
}

bool Window::tryLoadTexture(std::string tex)
{
    auto elem = std::find_if(m_textures.begin(), m_textures.end(), [&](const Texture &t) { return t.name == tex; });
    if (elem != m_textures.end()) return !elem->failed;

    std::cout << "INFO: Loading texture " << tex << std::endl;

    m_textures.push_back(Texture(tex));
    Texture &t = m_textures[m_textures.size()-1];

    tex = ResourceLocator::getPathPNG(tex);

    unsigned char * data = stbi_load(tex.c_str(), &t.w, &t.h, 0, 4);
    if (stbi_failure_reason()) {
        std::cout << "ERROR: Could not load texture " << tex << ", reason: " << stbi_failure_reason() << std::endl;
        t.failed = true;
        return false;
    }

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return true;
}

bool Window::initialize()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_REFRESH_RATE, 60);

    m_handle = glfwCreateWindow(virtual_width * 4, virtual_height * 4, "open-zl", nullptr, nullptr);

    if (m_handle == nullptr) {
        char const * desc[1024];
        glfwGetError(desc);
        std::cerr << "ERROR: Failed to create GLFW window; " << *desc << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetFramebufferSizeCallback(m_handle, framebufferResize);
    //    glfwSetCursorPosCallback(m_handle, mouse_move_callback);
    //    glfwSetScrollCallback(m_handle, mouse_wheel_callback);

    glfwMakeContextCurrent(m_handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "ERROR: Failed to initialize GLAD" << std::endl;
        return false;
    }

    glClearColor(0.05f, 0.03f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClear(GL_COLOR_BUFFER_BIT);

    // for now, we're only using one shader - the screenspace texture renderer
    //   if it turns out that we want more, then migrate this block to its own
    //   class, load shader source code properly, and add the shader uniform hooks
    {
        unsigned int vertexShader, fragmentShader;
        char failLog[512];
        int success;

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char * vsrc = "#version 330 core\nlayout (location = 0) in vec2 vpos;\nlayout (location = 1) in vec2 uv;\nout vec2 texCoord;\nuniform float xverts[2] = float[2](0, 1);\nuniform float yverts[2] = float[2](0, 1);\nuniform vec2 screenSize = vec2(256, 144);\nuniform vec2 topleft = vec2(0, 0);\nuniform vec2 size = vec2(20, 20);\nuniform float offset = 0;\nvoid main()\n{\nvec2 loc = (vpos + vec2(0.5, -0.5)) * size * 2 / screenSize;\nloc = loc + vec2(-1, 1);\nvec2 delta = topleft * 2 / screenSize;\nloc = loc + vec2(delta.x, -delta.y);\ngl_Position = vec4(loc, -0.95 - offset, 1.0);\ntexCoord = vec2(xverts[int(uv.x)], yverts[int(uv.y)]);\n}";

        glShaderSource(vertexShader, 1, &vsrc, nullptr);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, failLog);
            std::cerr << "ERROR: Could not compile vertex shader " << failLog;
            return false;
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char * fsrc = "#version 330 core\nin vec2 texCoord;\nout lowp vec4 FragColor;\nuniform sampler2D tex0;\nuniform vec4 colorMult = vec4(1, 1, 1, 1);\nvoid main()\n{\nvec4 c = texture(tex0, texCoord) * colorMult;\nif (c.a < 0.1) discard;\nFragColor = c;\n}";

        glShaderSource(fragmentShader, 1, &(fsrc), nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, failLog);
            std::cerr << "ERROR: Could not compile fragment shader " << failLog;
            return false;
        }

        m_texture_shader = glCreateProgram();
        glAttachShader(m_texture_shader, vertexShader);
        glAttachShader(m_texture_shader, fragmentShader);
        glLinkProgram(m_texture_shader);
        glGetProgramiv(m_texture_shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_texture_shader, 512, nullptr, failLog);
            std::cerr << "ERROR: Could not link shader " << failLog;
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glUseProgram(m_texture_shader);
    }

    // upload the vertex array that we'll use for drawing textures
    {
        float verts[] = {
             0.5f,  0.5f, 1, 0, // x y u v
             0.5f, -0.5f, 1, 1,
            -0.5f, -0.5f, 0, 1,
            -0.5f,  0.5f, 0, 0,
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3,
        };

        unsigned int vbo, ebo;
        glGenVertexArrays(1, &m_texture_vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(m_texture_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), verts, GL_STATIC_DRAW);

        // vertex attributes
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // uv mapping attributes
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    // create a 1x1 white texture (for use with pen actions)
    {
        glGenTextures(1, &m_pen_texture);
        glBindTexture(GL_TEXTURE_2D, m_pen_texture);
        unsigned char data[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    m_active_font = Font::load(MagoFont3);

    return true;
}

void Window::renderWindow()
{
    glfwSwapBuffers(m_handle);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_texture_shader);
}

void Window::setClip(float x1, float y1, float x2, float y2)
{
    float a[2];
    a[0] = x1;
    a[1] = x2;
    int loc = glGetUniformLocation(m_texture_shader, "xverts");
    glUniform1fv(loc, 2, a);

    a[0] = y1;
    a[1] = y2;
    loc = glGetUniformLocation(m_texture_shader, "yverts");
    glUniform1fv(loc, 2, a);
}

void Window::bindTexture(std::string texture_name, float w, float h)
{
    if (!tryLoadTexture(texture_name)) return;
    Texture &tex = *std::find_if(m_textures.begin(), m_textures.end(), [&](const Texture &t) { return t.name == texture_name; });

    {
        int loc = glGetUniformLocation(m_texture_shader, "size");
        glUniform2f(loc, w, h);

        loc = glGetUniformLocation(m_texture_shader, "colorMult");
        glUniform4f(loc, 1, 1, 1, 1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glBindVertexArray(m_texture_vao);
}

void Window::drawActive(float x, float y)
{
    {
        int loc = glGetUniformLocation(m_texture_shader, "topleft");
        glUniform2f(loc, std::floor(x), std::floor(y));
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Window::draw(std::string texture_name, float x, float y)
{
    if (!tryLoadTexture(texture_name)) return;
    Texture &tex = *std::find_if(m_textures.begin(), m_textures.end(), [&](const Texture &t) { return t.name == texture_name; });

    {
        int loc = glGetUniformLocation(m_texture_shader, "topleft");
        // TODO: see others' preference on sub pixel rendering; assumed undesireable
        glUniform2f(loc, std::floor(x), std::floor(y));

        loc = glGetUniformLocation(m_texture_shader, "size");
        glUniform2f(loc, float(tex.w), float(tex.h));

        loc = glGetUniformLocation(m_texture_shader, "colorMult");
        glUniform4f(loc, 1, 1, 1, 1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glBindVertexArray(m_texture_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Window::draw(std::string texture_name, float x, float y, float w, float h)
{
    if (!tryLoadTexture(texture_name)) return;
    Texture &tex = *std::find_if(m_textures.begin(), m_textures.end(), [&](const Texture &t) { return t.name == texture_name; });

    {
        int loc = glGetUniformLocation(m_texture_shader, "topleft");
        // TODO: see others' preference on sub pixel rendering; assumed undesireable
        glUniform2f(loc, std::floor(x), std::floor(y));

        loc = glGetUniformLocation(m_texture_shader, "size");
        glUniform2f(loc, w, h);

        loc = glGetUniformLocation(m_texture_shader, "colorMult");
        glUniform4f(loc, 1, 1, 1, 1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glBindVertexArray(m_texture_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Window::setPen(float r, float g, float b, float a)
{
    m_pen_set[0] = r;
    m_pen_set[1] = g;
    m_pen_set[2] = b;
    m_pen_set[3] = a;
}

void Window::rect(float x, float y, float w, float h)
{
    {
        int loc = glGetUniformLocation(m_texture_shader, "topleft");
        glUniform2f(loc, std::floor(x), std::floor(y));

        loc = glGetUniformLocation(m_texture_shader, "size");
        glUniform2f(loc, w, h);

        loc = glGetUniformLocation(m_texture_shader, "colorMult");
        glUniform4f(loc, m_pen_set[0], m_pen_set[1], m_pen_set[2], m_pen_set[3]);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_pen_texture);

    glBindVertexArray(m_texture_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Window::print(std::string text, float x, float y, float scale)
{
    m_active_font.bind();

    int px = std::floor(x);

    {
        int loc = glGetUniformLocation(m_texture_shader, "size");
        glUniform2f(loc, scale * float(m_active_font.getGlyphWidth()), scale * float(m_active_font.getGlyphHeight()));

        loc = glGetUniformLocation(m_texture_shader, "colorMult");
        glUniform4f(loc, m_pen_set[0], m_pen_set[1], m_pen_set[2], m_pen_set[3]);
    }

    int uvx_loc = glGetUniformLocation(m_texture_shader, "xverts");
    int uvy_loc = glGetUniformLocation(m_texture_shader, "yverts");

    float uvs[4];
    for(const char& ch : text) {
        m_active_font.getGlyphUVs(ch, uvs);
        glUniform1fv(uvx_loc, 2, uvs);
        glUniform1fv(uvy_loc, 2, uvs + 2);
        drawActive(px, y);
        px += scale * float(m_active_font.getGlyphWidth(ch) + 1);
    }
    uvs[0] = 0;
    uvs[1] = 1;
    uvs[2] = 0;
    uvs[3] = 1;
    glUniform1fv(uvx_loc, 2, uvs);
    glUniform1fv(uvy_loc, 2, uvs + 2);
}
