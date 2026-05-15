#pragma once

#include "scenes/IScene.hpp"
#include <vector>

namespace scenes {

enum class LanHostMenuItem {
    Cancel
};

class LanHostScene : public IScene {

public:
    explicit LanHostScene();

    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    std::vector<renderer::Button> m_buttons;
};

}
