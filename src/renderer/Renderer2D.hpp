#pragma once

#include "scenes/SceneManager.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace renderer {

static constexpr float DEFAULT_FONT_SIZE = 64.0f;
static constexpr int DEFAULT_CIRCLE_SEGMENTS = 32;
static constexpr float DEFAULT_RECT_THICKNESS = 0.05f;

class Shader;
class Buffer;
class TextRenderer;

class Renderer2D {

public:
    Renderer2D();
    ~Renderer2D();

    Renderer2D(const Renderer2D&) = delete;
    Renderer2D& operator=(const Renderer2D&) = delete;

    void load_font(const std::string& path, float size_px = DEFAULT_FONT_SIZE);

    void draw_quad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);

    void draw_circle(const glm::vec2& center, float radius, const glm::vec4& color, int segments = DEFAULT_CIRCLE_SEGMENTS);

    void draw_rect_outline(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, float thickness = DEFAULT_RECT_THICKNESS);

    void draw_text(const std::string& text, float x, float y, float scale, const glm::vec4& color);

    const glm::mat4& projection() const { return m_proj; }

private:
    glm::mat4 m_proj;

    std::unique_ptr<Shader> m_shape_shader;
    std::unique_ptr<Buffer> m_shape_buffer;
    std::unique_ptr<Shader> m_text_shader;
    std::unique_ptr<Buffer> m_text_buffer;
    std::unique_ptr<TextRenderer> m_text_renderer;

};

}
