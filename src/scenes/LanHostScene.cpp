#include "LanHostScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"

namespace scenes {

LanHostScene::LanHostScene() {

    m_buttons.push_back({static_cast<int>(LanHostMenuItem::Cancel), "Cancel", {0.0f, 0.0f}, {renderer::Button::DEFAULT_W, renderer::Button::DEFAULT_H}});
}

std::string LanHostScene::update(const core::FrameInput& input, float dt) {
    (void)dt;
    const auto n  = static_cast<int>(m_buttons.size());
    const auto& p = input.players[0];

    int sel = -1;
    for (int i = 0; i < n; ++i) if (m_buttons[i].selected) { sel = i; break; }

    if (sel == -1 && (p.nav_up || p.nav_down)) {
        m_buttons[0].selected = true;
        sel = 0;
    }

    if (input.mouse.moved) {
        for (auto& btn : m_buttons)
            btn.handle_hover({input.mouse.x, input.mouse.y});
    }

    for (int i = 0; i < n; ++i)
        m_buttons[i].hovered = m_buttons[i].selected;

    if (p.confirm && sel >= 0) return Transition::MainMenu;

    if (input.mouse.left_pressed) {
        for (auto& btn : m_buttons) {
            if (btn.handle_click({input.mouse.x, input.mouse.y}))
                return Transition::MainMenu;
        }
    }

    if (p.back) return Transition::MainMenu;

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
