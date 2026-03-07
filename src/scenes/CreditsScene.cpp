#include "CreditsScene.hpp"
#include "core/InputState.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/Colors.hpp"
#include "scenes/IScene.hpp"

namespace scenes {

std::string CreditsScene::update(const core::InputState& input, float dt) {
    (void)dt;

    if (input.is_pressed(core::Key::Escape) || input.mouse.left_pressed)
        return Transition::MainMenu;

    return Transition::Stay;
}

void CreditsScene::render(renderer::Renderer2D& r) const {
    const glm::vec2 bg_pos  = {0.0f, 0.0f};
    const glm::vec2 bg_size = {14.0f, 10.0f};

    r.draw_quad(bg_pos, bg_size, {0.05f, 0.05f, 0.12f, 1.0f});
    r.draw_rect_outline(bg_pos, bg_size, {0.4f, 0.4f, 0.8f, 0.8f}, 0.05f);

    r.draw_text("CREDITS",                 0.0f, 4.0f, 1.4f , Colors::CreditsNormal);

    r.draw_text("Developer",               0.0f, 2.4f, 0.55f, Colors::CreditsAccent);
    r.draw_text("Andrea D'Addabbo",        0.0f, 1.7f, 0.5f , Colors::CreditsNormal);

    r.draw_text("Libraries",               0.0f, 0.7f, 0.55f, Colors::CreditsAccent);
    r.draw_text("SDL3",                    0.0f, 0.1f, 0.45f, Colors::CreditsSubtle);
    r.draw_text("OpenGL",                  0.0f,-0.5f, 0.45f, Colors::CreditsSubtle);
    r.draw_text("GLAD + GLM",              0.0f,-1.1f, 0.45f, Colors::CreditsSubtle);

    r.draw_text("Click or ESC to go back", 0.0f,-4.2f, 0.45f, {0.5f, 0.5f, 0.5f, 1.0f});
}

} // namespace scenes
