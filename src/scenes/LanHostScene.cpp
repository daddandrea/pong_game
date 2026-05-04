#include "LanHostScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"
#include <array>

namespace scenes {

LanHostScene::LanHostScene(game::GameConfig& config)
    : m_config(config) {

    struct Entry {
        LanHostMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries {
        Entry{LanHostMenuItem::Cancel, "Cancel", 0.0f}
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

std::string LanHostScene::update(const core::InputState &input, float dt) {
    (void)dt;
    (void)input;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {

            if (!btn.hovered) continue;

            if (static_cast<LanHostMenuItem>(btn.item) == LanHostMenuItem::Cancel) {
                return Transition::MainMenu;
            }
        }
    }

    //TODO: Make network server hosting logic and wait for a client to connect.
    //      Also set the inputs

    return Transition::Stay;
}

void LanHostScene::render(renderer::Renderer2D& r) const {
    r.draw_text("Hosting on IP...", 0.0f, 1.2f, 1.0f, Colors::MainSubtle);

    for (const auto& btn : m_buttons) {
        r.draw_button(btn);
    }

    //TODO: Change 1 == 2 to logic when ip is connected.
    if (1 == 2) {
        r.draw_text("IP is connected! Starting the game now.", 0.0f, -1.2f, 1.0f, Colors::MainSubtle);
    }
}

}
