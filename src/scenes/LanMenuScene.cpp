#include "LanMenuScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"
#include <array>

namespace scenes {

LanMenuScene::LanMenuScene(game::GameConfig& config)
    : m_config(config) {

    using enum LanMenuItem;

    struct Entry {
        LanMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries = {
        Entry{Host,     "Host a game",  1.2f},
        Entry{Join,     "Join a game",  0.0f},
        Entry{MainMenu, "Main Menu",   -1.2f},
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

std::string LanMenuScene::update(const core::InputState& input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {

            if (!btn.hovered) continue;

            switch (static_cast<LanMenuItem>(btn.item)) {
                using enum LanMenuItem;

                case Host:
                    return Transition::LanHost;

                case Join:
                    return Transition::LanJoin;

                case MainMenu:
                    return Transition::MainMenu;
            }
        }
    }

    return Transition::Stay;
}

void LanMenuScene::render(renderer::Renderer2D& r) const {
    r.draw_text("PONG", 0.0f, 4.5f, 2.5f, Colors::MainWhite);
    r.draw_text("OpenGL learning experiment", 0.0f, 3.2f, 0.55f, Colors::MainSubtle);
    r.draw_text("v" PONG_VERSION, 0.0f, -5.7f, 0.4f, Colors::MainSubtle);

    for (const auto& btn : m_buttons) {
        r.draw_button(btn);
    }
}

}
