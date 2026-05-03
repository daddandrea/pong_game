#include "MultiPlayerMenuScene.hpp"
#include "core/InputState.hpp"
#include "game/GameConfig.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/IScene.hpp"
#include "scenes/Colors.hpp"
#include <array>

namespace scenes {

MultiPlayerMenuScene::MultiPlayerMenuScene(game::GameConfig& config)
    : m_config(config) {

    using enum MultiPlayerMenuItem;

    struct Entry {
        MultiPlayerMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries = {
        Entry{Local,    "Local Multiplayer",   1.2f},
        Entry{Online,   "Online Multiplayer",  0.0f},
        Entry{MainMenu, "Main Menu",          -1.2f},
    };

    for (auto& e : entries) {
        m_buttons.push_back(
            {
                static_cast<int>(e.item),
                e.label,
                {0.0f, e.y},
                {BTN_W, BTN_H}
            }
        );
    }
}

std::string MultiPlayerMenuScene::update(const core::InputState& input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {

            if (!btn.hovered) continue;

            switch (static_cast<MultiPlayerMenuItem>(btn.item)) {
                using enum game::PlayerType;

                case MultiPlayerMenuItem::Local:
                    m_config.left  = Human;
                    m_config.right = Human;
                    return Transition::Game;

                case MultiPlayerMenuItem::Online:
                    m_config.left = Human;
                    return Transition::MatchMaking;

                case MultiPlayerMenuItem::MainMenu:
                    return Transition::MainMenu;
            }
        }
    }

    return Transition::Stay;
}

void MultiPlayerMenuScene::render(renderer::Renderer2D& r) const {
    r.draw_text("PONG", 0.0f, 4.5f, 2.5f, Colors::MainWhite);
    r.draw_text("OpenGL learning experiment", 0.0f, 3.2f, 0.55f, Colors::MainSubtle);
    r.draw_text("v" PONG_VERSION, 0.0f, -5.7f, 0.4f, Colors::MainSubtle);

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
