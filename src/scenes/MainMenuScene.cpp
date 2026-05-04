#include "MainMenuScene.hpp"
#include "core/InputState.hpp"
#include "renderer/Renderer2D.hpp"
#include "renderer/Button.hpp"
#include "scenes/Colors.hpp"
#include <array>

#ifndef PONG_VERSION
#define PONG_VERSION "0.0.0"
#endif


namespace scenes {

MainMenuScene::MainMenuScene(game::GameConfig& config)
    : m_config(config) {

    using enum MainMenuItem;

    struct Entry {
        MainMenuItem item;
        const char* label;
        float y;
    };

    const std::array entries = {
        Entry{SinglePlayer, "Singleplayer",     1.2f},
        Entry{MultiPlayer,  "Multiplayer",  0.0f},
        Entry{Credits,      "Credits",          -1.2f},
        Entry{Quit,         "Quit",             -2.6f},
    };

    for (const auto& e : entries) {
        m_buttons.push_back({static_cast<int>(e.item), e.label, {0.0f, e.y}, {BTN_W, BTN_H}});
    }
}

std::string MainMenuScene::update(const core::InputState& input, float dt) {
    (void)dt;

    for (auto& btn : m_buttons) {
        btn.hovered = btn.contains( {input.mouse.x, input.mouse.y} );
    }

    if (input.mouse.left_pressed) {
        for (const auto& btn : m_buttons) {
            if (!btn.hovered) continue;

            switch (static_cast<MainMenuItem>(btn.item)) {
                using enum game::PlayerType;

                case MainMenuItem::SinglePlayer:
                    return Transition::SinglePlayer;

                case MainMenuItem::MultiPlayer:
                    return Transition::MultiPlayer;

                case MainMenuItem::Credits:
                    return Transition::Credits;

                case MainMenuItem::Quit:
                    return Transition::Quit;

            }
        }
    }

    if (input.is_pressed(core::Key::Escape)) return Transition::Quit;

    return Transition::Stay;
}

void MainMenuScene::render(renderer::Renderer2D& r) const {
    r.draw_text("PONG", 0.0f, 4.5f, 2.5f, Colors::MainWhite);
    r.draw_text("OpenGL learning experiment", 0.0f, 3.2f, 0.55f, Colors::MainSubtle);
    r.draw_text("v" PONG_VERSION, 0.0f, -5.7f, 0.4f, Colors::MainSubtle);

    for (const auto& btn : m_buttons) {
        r.draw_button(btn);
    }
}

}
