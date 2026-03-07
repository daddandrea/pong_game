#pragma once

#include <string>
#include <glm/glm.hpp>

struct TTF_Font;

namespace renderer {

class Shader;
class Buffer;

class TextRenderer {
public:
    TextRenderer(Shader& shader, Buffer& buffer);
    ~TextRenderer();

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    void load_font(const std::string& path, float size_px);

    void draw(const std::string& text,
              float x,
              float y,
              float scale,
              const glm::vec4& color,
              const glm::mat4& proj);

private:
    Shader& m_shader;
    Buffer& m_buffer;
    TTF_Font* m_font = nullptr;
};

}
