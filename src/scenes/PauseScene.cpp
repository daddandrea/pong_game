#include "PauseScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"

namespace scenes {

PauseScene::PauseScene(game::DevSettings& dev_settings)
    : m_dev(dev_settings) {

    m_buttons.push_back({
        static_cast<int>(PauseItem::Resume),
        "Resume",
        {0.0f, 0.6f},
        {renderer::Button::PAUSE_W, renderer::Button::PAUSE_H}
    });
    m_buttons.push_back({
        static_cast<int>(PauseItem::MainMenu),
        "Main Menu",
        {0.0f,-0.6f},
        {renderer::Button::PAUSE_W, renderer::Button::PAUSE_H}
    });
    m_buttons.push_back({
        static_cast<int>(PauseItem::Quit),
        "Quit",
        {0.0f,-1.8f},
        {renderer::Button::PAUSE_W, renderer::Button::PAUSE_H}
    });
}

std::string PauseScene::update(const core::FrameInput& input, float dt) {
    (void)dt;
    const int n   = static_cast<int>(m_buttons.size());
    const auto& p = input.players[0];

    if (p.back) return Transition::Pop;

    int sel = -1;
    for (int i = 0; i < n; ++i) if (m_buttons[i].selected) { sel = i; break; }

    if (sel == -1) {
        if (p.nav_up || p.nav_down) {
            m_buttons[0].selected = true;
            sel = 0;
        }
    } else {
        if (p.nav_up) {
            m_buttons[sel].selected = false;
            sel = (sel - 1 + n) % n;
            m_buttons[sel].selected = true;
        }
        if (p.nav_down) {
            m_buttons[sel].selected = false;
            sel = (sel + 1) % n;
            m_buttons[sel].selected = true;
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

    auto activate = [&](int idx) -> std::string {
        switch (static_cast<PauseItem>(m_buttons[idx].item)) {
            case PauseItem::Resume:   return Transition::Pop;
            case PauseItem::MainMenu: return Transition::MainMenu;
            case PauseItem::Quit:     return Transition::Quit;
        }
        return Transition::Stay;
    };

    if (p.confirm && sel >= 0) return activate(sel);

    if (input.mouse.left_pressed) {
        for (int i = 0; i < n; ++i) {
            if (m_buttons[i].contains({input.mouse.x, input.mouse.y})) {
                if (sel >= 0) m_buttons[sel].selected = false;
                m_buttons[i].selected = true;
                return activate(i);
            }
        }
    }

    return Transition::Stay;
}

void PauseScene::render(renderer::Renderer2D& r) const {
    r.draw_quad({0.0f, 0.0f}, {20.0f, 12.0f}, {0.0f, 0.0f, 0.0f, 0.65f}); //Semi-trasparent overlay

    r.draw_quad({0.0f, 0.0f}, {6.5f, 6.0f}, {0.08f, 0.08f, 0.15f, 0.95f});
    r.draw_rect_outline({0.0f, 0.0f}, {6.5, 6.0f}, {0.4f, 0.4f, 0.8f, 1.0f});

    r.draw_text("PAUSED", 0.0f, 2.3f, 1.3f, Colors::MainWhite);

    for (const auto& btn : m_buttons) {
        r.draw_button(btn);
    }
}

} // namespace scenes
