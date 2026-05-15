#pragma once

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
    explicit MainMenuScene();

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    std::vector<renderer::Button> m_buttons;


};

}
