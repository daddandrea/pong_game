#pragma once

#include "scenes/IScene.hpp"
#include <vector>

namespace scenes {

enum class LanMenuItem {
    Host,
    Join,
    MainMenu
};

class LanMenuScene : public IScene {

public:
    explicit LanMenuScene(game::GameConfig& config);

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;

private:
    game::GameConfig& m_config;
    std::vector<renderer::Button> m_buttons;

};


}
