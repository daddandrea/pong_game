#pragma once

#include "IScene.hpp"
#include "core/InputState.hpp"
#include "game/GameConfig.hpp"
#include <vector>

namespace scenes {

enum class SinglePlayerMenuItem {
    PlayerVsCpu,
    CpuVsCpu,
    MainMenu
};

class SinglePlayerMenuScene : public IScene {

public:
    explicit SinglePlayerMenuScene(game::GameConfig& config);

    std::string update(const core::InputState &input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    game::GameConfig& m_config;

    std::vector<Button> m_buttons;

    static constexpr float BTN_W = 5.0f;
    static constexpr float BTN_H = 0.9f;

};

}
