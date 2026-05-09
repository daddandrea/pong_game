#include "LanJoinScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"

namespace scenes {

LanJoinScene::LanJoinScene(game::GameConfig& config)
    : m_config(config) {

    m_buttons.push_back({static_cast<int>(LanJoinMenuItem::MainMenu), "Main Menu", {0.0f, -1.2f}, {renderer::Button::DEFAULT_W, renderer::Button::DEFAULT_H}});
}

std::string LanJoinScene::update(const core::FrameInput& input, float dt) {
    (void)dt;
    const int n   = static_cast<int>(m_buttons.size());
    const auto& p = input.players[0];

    int sel = -1;
    for (int i = 0; i < n; ++i) if (m_buttons[i].selected) { sel = i; break; }

    if (sel == -1) {
        if (p.nav_up || p.nav_down) {
            m_buttons[0].selected = true;
            sel = 0;
        }
    }

    if (input.mouse.moved) {
        for (int i = 0; i < n; ++i) {
            if (m_buttons[i].contains({input.mouse.x, input.mouse.y})) {
                if (sel >= 0) m_buttons[sel].selected = false;
                m_buttons[i].selected = true;
                sel = i;
            }
        }
    }

    for (int i = 0; i < n; ++i) m_buttons[i].hovered = m_buttons[i].selected;

    if (p.confirm && sel >= 0) return Transition::MainMenu;

    if (input.mouse.left_pressed) {
        for (int i = 0; i < n; ++i) {
            if (m_buttons[i].contains({input.mouse.x, input.mouse.y})) {
                return Transition::MainMenu;
            }
        }
    }

    if (p.back) return Transition::MainMenu;

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
