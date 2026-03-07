#include "TextRenderer.hpp"
#include "renderer/Buffer.hpp"
#include "renderer/Shader.hpp"
#include "renderer/ShaderUniforms.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdexcept>
#include <string>

namespace renderer {

TextRenderer::TextRenderer(Shader& shader, Buffer& buffer) 
    : m_shader(shader), m_buffer(buffer) {}

TextRenderer::~TextRenderer() {
    if (m_font) TTF_CloseFont(m_font);
}

void TextRenderer::load_font(const std::string& path, float size_px) {
    if (m_font) TTF_CloseFont(m_font);

    m_font = TTF_OpenFont(path.c_str(), size_px);
    if (!m_font) {
        throw std::runtime_error(std::string("TTF_OpenFont failed(" + path + "): " + SDL_GetError()));
    }
}

void TextRenderer::draw(const std::string& text,
                        float x,
                        float y,
                        float scale,
                        const glm::vec4& color,
                        const glm::mat4& proj) {
    if (!m_font || text.empty()) return;

    SDL_Color sdl_col = {
        static_cast<Uint8>(color.r * 255.0f),
        static_cast<Uint8>(color.g * 255.0f),
        static_cast<Uint8>(color.b * 255.0f),
        static_cast<Uint8>(color.a * 255.0f),
    };

    SDL_Surface* raw = TTF_RenderText_Blended(m_font, text.c_str(), 0, sdl_col);
    if (!raw) return;

    SDL_Surface* surf = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(raw);
    if (!surf) return;

    const float font_h = static_cast<float>(surf->h);
    const float world_h = scale;
    const float world_w = scale * (static_cast<float>(surf->w) / font_h);

    const float x0 = x - world_w / 2;
    const float x1 = x + world_w / 2;
    const float y0 = y + world_h / 2;
    const float y1 = y - world_h / 2;

    GLuint tex = 0;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 surf->w,
                 surf->h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 surf->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_DestroySurface(surf);

    const std::vector<float> verts = {
        x0, y0, 0.0f, 0.0f,
        x1, y0, 1.0f, 0.0f,
        x0, y1, 0.0f, 1.0f,
        x1, y0, 1.0f, 0.0f,
        x1, y1, 1.0f, 1.0f,
        x0, y1, 0.0f, 1.0f,
    };

    m_shader.bind();
    m_shader.set_mat4(Uniforms::Proj, proj);
    m_shader.set_int(Uniforms::Texture, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    m_buffer.upload(verts, 4);
    m_buffer.draw(GL_TRIANGLES, 6);

    glDeleteTextures(1, &tex);
}

}
