#include "MatchMakingScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/Colors.hpp"
#include "scenes/IScene.hpp"
#include <array>

namespace scenes {

MatchMakingScene::MatchMakingScene(game::GameConfig& config)
    : m_config(config) {

    using enum MatchMakingMenuItem;

    struct Entry {
        MatchMakingMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries = {
        Entry{Cancel, "Cancel", 0.0f}
    };

    for (const auto& e : entries) {
        m_buttons.push_back({
            static_cast<int>(e.item),
            e.label,
            {0.0f, e.y},
            {BTN_W, BTN_H}
        });
    }

}

std::string MatchMakingScene::update(const core::InputState &input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {

            if (!btn.hovered) continue;

            if (static_cast<MatchMakingMenuItem>(btn.item) == MatchMakingMenuItem::Cancel) {
                return Transition::MainMenu;
            }
        }
    }

    return Transition::Stay;
}

void MatchMakingScene::render(renderer::Renderer2D& r) const {
    r.draw_text("Searching for match...", 0.0f, 3.2f, 0.55f, Colors::MainSubtle);
    r.draw_text("v" PONG_VERSION,         0.0f, -5.7f, 0.4f, Colors::MainSubtle);

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
