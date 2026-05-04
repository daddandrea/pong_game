#include "LanJoinScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"
#include <array>

namespace scenes {

LanJoinScene::LanJoinScene(game::GameConfig& config)
    : m_config(config) {

    struct Entry {
        LanJoinMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries = {
        Entry{LanJoinMenuItem::MainMenu, "MainMenu", -1.2f}
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

std::string LanJoinScene::update(const core::InputState &input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {

            if (!btn.hovered) continue;

            if (static_cast<LanJoinMenuItem>(btn.item) == LanJoinMenuItem::MainMenu) {
                return Transition::MainMenu;
            }
        }
    }

    return Transition::Stay;
}

void LanJoinScene::render(renderer::Renderer2D &r) const {
    //TODO: Add logic to get the number of matches
    int n_of_matches = 0;

    if (n_of_matches == 0) {
        r.draw_text("No matches found",       0.0f, 1.2f, 1.0f, Colors::MainSubtle);
    } else {
        r.draw_text("Choose a match to join", 0.0f, 1.2f, 1.0f, Colors::MainSubtle);
    }

    for (const auto& btn : m_buttons) {
        r.draw_button(btn);
    }

}

}
