#include "MainMenuScene.hpp"
#include "core/InputState.hpp"
#include "game/GameConfig.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/Colors.hpp"
#include "scenes/IScene.hpp"


namespace scenes {
// ── Button ────────────────────────────────────────────────────────────────────
bool Button::contains(glm::vec2 pt) const {
    return pt.x >= center.x - size.x / 2.0f
        && pt.x <= center.x + size.x / 2.0f
        && pt.y >= center.y - size.y / 2.0f
        && pt.y <= center.y + size.y / 2.0f;
}

// ── MainMenuScene ─────────────────────────────────────────────────────────────
MainMenuScene::MainMenuScene() {
    struct Entry {
        MenuItem item;
        const char* label;
        float y;
    };

    const Entry entries[] = {
        {MenuItem::PlayerVsCpu   , "Player vs CPU",     1.2f},
        {MenuItem::PlayerVsPlayer, "Player vs Player",  0.0f},
        {MenuItem::CpuVsCpu      , "CPU vs CPU",       -1.2f},
        {MenuItem::Credits       , "Credits",          -2.6f},
        {MenuItem::Quit          , "Quit",             -3.8f},
    };

    for (const auto& e : entries) {
        m_buttons.push_back({static_cast<int>(e.item), e.label, {0.0f, e.y}, {BTN_W, BTN_H}});
    }
}

std::string MainMenuScene::update(const core::InputState& input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains( {input.mouse.x, input.mouse.y} );
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {
            if (!btn.hovered) continue;

            switch (static_cast<MenuItem>(btn.item)) {
                case MenuItem::PlayerVsCpu:
                    game::g_config.left = game::PlayerType::Human;
                    game::g_config.right = game::PlayerType::Cpu;
                    return Transition::Game;

                case MenuItem::PlayerVsPlayer:
                    game::g_config.left = game::PlayerType::Human;
                    game::g_config.right = game::PlayerType::Human;
                    return Transition::Game;

                case MenuItem::CpuVsCpu:
                    game::g_config.left = game::PlayerType::Cpu;
                    game::g_config.right = game::PlayerType::Cpu;
                    return Transition::Game;

                case MenuItem::Credits:
                    return Transition::Credits;

                case MenuItem::Quit:
                    return Transition::Quit;
            }
        }
    }

    if (input.is_pressed(core::Key::Escape)) return Transition::Quit;

    return Transition::Stay;
}

void MainMenuScene::render(renderer::Renderer2D& r) const {
    r.draw_text("PONG", 0.0f, 4.5f, 2.5f, Colors::MainWhite);
    r.draw_text("OpenGL learning experiment", 0.0f, 3.2f, 0.55f, Colors::MainSubtle);

    for (const auto& btn : m_buttons) {
        const glm::vec4 col = btn.hovered
                                ? Colors::BtnBgHovered
                                : Colors::BtnBgNormal;

        r.draw_quad(btn.center, btn.size, col);
        r.draw_rect_outline(btn.center, btn.size, Colors::BtnBorder, 0.04f);

        const glm::vec4 text_col = btn.hovered
                                     ? Colors::BtnTextHovered
                                     : Colors::BtnTextNormal;
        r.draw_text(btn.label, btn.center.x, btn.center.y, 0.5, text_col);
    }
}

}
