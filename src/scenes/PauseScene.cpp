#include "PauseScene.hpp"
#include "core/InputState.hpp"
#ifdef PONG_DEV
#include "game/DevSettings.hpp"
#endif
#include "renderer/Renderer2D.hpp"
#include "scenes/Colors.hpp"
#include "scenes/IScene.hpp"
#include "scenes/MainMenuScene.hpp"

namespace scenes {

PauseScene::PauseScene() {
    m_buttons.push_back({
        static_cast<int>(PauseItem::Resume),
        "Resume",
        {0.0f, 0.6f},
        {BTN_W, BTN_H}
    });
    m_buttons.push_back({
        static_cast<int>(PauseItem::MainMenu),
        "Main Menu",
        {0.0f,-0.6f},
        {BTN_W, BTN_H}
    });
    m_buttons.push_back({
        static_cast<int>(PauseItem::Quit),
        "Quit",
        {0.0f,-1.8f},
        {BTN_W, BTN_H}
    });
}

std::string PauseScene::update(const core::InputState& input, float dt) {
    (void)dt;

    if (input.is_pressed(core::Key::Escape)) return Transition::Pop;
#ifdef PONG_DEV
    if (input.is_pressed(core::Key::F1)) game::g_dev.show_dev = !game::g_dev.show_dev;
#endif

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains({input.mouse.x, input.mouse.y});
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {
            if (!btn.hovered) continue;

            switch (static_cast<PauseItem>(btn.item)) {
                case PauseItem::Resume:   return Transition::Pop;
                case PauseItem::MainMenu: return Transition::MainMenu;
                case PauseItem::Quit:     return Transition::Quit;
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
        const glm::vec4 col = btn.hovered
                                ? Colors::BtnBgHovered
                                : Colors::BtnBgNormal;
        r.draw_quad(btn.center, btn.size, col);
        r.draw_rect_outline(btn.center, btn.size, Colors::BtnBorder, 0.04f);

        const glm::vec4 text_col = btn.hovered
                               ? Colors::BtnTextHovered
                               : Colors::BtnTextNormal;
        r.draw_text(btn.label, btn.center.x, btn.center.y, 0.5f, text_col);
    }
}

} // namespace scenes
