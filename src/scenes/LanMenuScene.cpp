#include "LanMenuScene.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"

#ifndef PONG_VERSION
#define PONG_VERSION "0.0.0"
#endif

namespace scenes {

LanMenuScene::LanMenuScene() {
    using enum LanMenuItem;

    m_buttons.push_back({
        static_cast<int>(Host),
        "Host a game",
        {0.0f,  1.2f},
        {renderer::Button::DEFAULT_W, renderer::Button::DEFAULT_H}
    });

    m_buttons.push_back({
        static_cast<int>(Join),
        "Join a game",
        {0.0f,  0.0f},
        {renderer::Button::DEFAULT_W, renderer::Button::DEFAULT_H}
    });

    m_buttons.push_back({
        static_cast<int>(MainMenu),
        "Main Menu",
        {0.0f, -1.2f},
        {renderer::Button::DEFAULT_W, renderer::Button::DEFAULT_H}
    });
}

std::string LanMenuScene::update(const core::FrameInput& input, float dt) {
    (void)dt;
    const auto n  = static_cast<int>(m_buttons.size());
    const auto& p = input.players[0];

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
        renderer::select_hovered(m_buttons, sel, {input.mouse.x, input.mouse.y});
    }

    for (int i = 0; i < n; ++i) m_buttons[i].hovered = m_buttons[i].selected;

    auto activate = [&](int idx) -> std::string {
        using enum LanMenuItem;
        switch (static_cast<LanMenuItem>(m_buttons[idx].item)) {
            case Host:     return Transition::LanHost;
            case Join:     return Transition::LanJoin;
            case MainMenu: return Transition::MainMenu;
        }
        return Transition::Stay;
    };

    if (p.confirm && sel >= 0) return activate(sel);

    if (input.mouse.left_pressed) {
        if (int i = renderer::select_hovered(m_buttons, sel, {input.mouse.x, input.mouse.y}))
            return activate(i);
    }

    if (p.back) return Transition::MainMenu;

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
