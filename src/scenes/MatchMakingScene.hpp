#pragma once

#include "scenes/IScene.hpp"
#include <vector>

namespace scenes {

enum class MatchMakingMenuItem {
    Cancel
};

class MatchMakingScene : public IScene {

public:
    explicit MatchMakingScene();

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;

private:
    std::vector<renderer::Button> m_buttons;
};

}
