#pragma once

#include "game/DevSettings.hpp"
#include "scenes/IScene.hpp"
#include <vector>

namespace scenes {

enum class PauseItem { Resume, MainMenu, Quit };

class PauseScene : public IScene {

public:
    explicit PauseScene(game::DevSettings& dev_settings);

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;

private:
    game::DevSettings& m_dev;
    std::vector<renderer::Button> m_buttons;
};

} // namespace scenes
