#pragma once

#include "game/GameConfig.hpp"
#include "scenes/IScene.hpp"

#include <string>
#include <vector>

namespace scenes {

enum class MainMenuItem {
    SinglePlayer,
    MultiPlayer,
    Credits,
    Quit
};

class MainMenuScene : public IScene {

public:
    explicit MainMenuScene(game::GameConfig& config);

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    game::GameConfig& m_config;

    std::vector<renderer::Button> m_buttons;


};

}
