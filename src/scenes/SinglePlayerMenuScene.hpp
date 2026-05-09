#pragma once

#include "IScene.hpp"
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

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    game::GameConfig& m_config;

    std::vector<renderer::Button> m_buttons;


};

}
