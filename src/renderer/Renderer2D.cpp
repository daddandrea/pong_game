#include "Renderer2D.hpp"
#include "renderer/Buffer.hpp"
#include "renderer/Shader.hpp"
#include "renderer/ShaderUniforms.hpp"
#include "renderer/TextRenderer.hpp"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <glm/ext/matrix_clip_space.hpp>

#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

namespace renderer {

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot open shader file: " + path);

    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();

}

Renderer2D::Renderer2D() {
    m_proj = glm::ortho(-10.0f, 10.0f, -6.0f, 6.0f, -1.0f, 1.0f);

    const std::string base       = SDL_GetBasePath();
    const std::string shaders    = base + "assets/shaders/";

    const std::string shape_vert = read_file(shaders + "shape.vert");
    const std::string shape_frag = read_file(shaders + "shape.frag");
    const std::string text_vert  = read_file(shaders + "text.vert");
    const std::string text_frag  = read_file(shaders + "text.frag");

    m_shape_shader = std::make_unique<Shader>(shape_vert.c_str(), shape_frag.c_str());
    m_shape_buffer = std::make_unique<Buffer>();

    m_text_shader = std::make_unique<Shader>(text_vert.c_str(), text_frag.c_str());
    m_text_buffer = std::make_unique<Buffer>();
    m_text_renderer = std::make_unique<TextRenderer>(*m_text_shader, *m_text_buffer);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Renderer2D::~Renderer2D() = default;

void Renderer2D::load_font(const std::string& path, float size_px) {
    m_text_renderer->load_font(path, size_px);
}

void Renderer2D::draw_quad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
    const float hw = size.x / 2;
    const float hh = size.y / 2;
    const float x0 = pos.x - hw;
    const float x1 = pos.x + hw;
    const float y0 = pos.y - hh;
    const float y1 = pos.y + hh;

    const std::vector<float> verts = {
        x0, y0,
        x1, y0,
        x0, y1,
        x1, y0,
        x1, y1,
        x0, y1,
    };

    m_shape_shader->bind();
    m_shape_shader->set_mat4(Uniforms::Proj, m_proj);
    m_shape_shader->set_vec4(Uniforms::Color, color);
    m_shape_buffer->upload(verts, 2);
    m_shape_buffer->draw(GL_TRIANGLES, 6);
}

void Renderer2D::draw_circle(const glm::vec2& center, float radius, const glm::vec4& color, int segments) {
    std::vector<float> verts;
    verts.reserve(static_cast<size_t>(segments) * 6);

    const float step = 2.0f * static_cast<float>(M_PI) / static_cast<float>(segments);

    for (int i = 0; i < segments; ++i) {
        const float a0 = static_cast<float>(i) * step;
        const float a1 = static_cast<float>(i + 1) * step;

        verts.push_back(center.x);
        verts.push_back(center.y);
        verts.push_back(center.x + std::cos(a0) * radius);
        verts.push_back(center.y + std::sin(a0) * radius);
        verts.push_back(center.x + std::cos(a1) * radius);
        verts.push_back(center.y + std::sin(a1) * radius);
    }

    m_shape_shader->bind();
    m_shape_shader->set_mat4(Uniforms::Proj, m_proj);
    m_shape_shader->set_vec4(Uniforms::Color, color);
    m_shape_buffer->upload(verts, 2);
    m_shape_buffer->draw(GL_TRIANGLES, segments * 3);
}

void Renderer2D::draw_rect_outline(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color, float thickness) {
    const float hw = size.x / 2;
    const float hh = size.y / 2;

    draw_quad({pos.x, pos.y + hh - thickness / 2}, {size.x, thickness}, color); //TOP
    draw_quad({pos.x, pos.y - hh + thickness / 2}, {size.x, thickness}, color); //BOTTOM
    draw_quad({pos.x - hw + thickness / 2, pos.y}, {thickness, size.y}, color); //LEFT
    draw_quad({pos.x + hw - thickness / 2, pos.y}, {thickness, size.y}, color); //RIGHT
}

void Renderer2D::draw_text(const std::string& text, float x, float y, float scale, const glm::vec4& color) {
    m_text_renderer->draw(text, x, y, scale, color, m_proj);
}
}
