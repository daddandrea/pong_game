#pragma once

#include "IScene.hpp"

namespace scenes {

class CreditsScene : public IScene {
public:
    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;
};

} // namespace scenes
