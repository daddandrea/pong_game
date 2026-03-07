#pragma once

#include "IScene.hpp"
#include "core/InputState.hpp"

namespace scenes {

class CreditsScene : public IScene {
public:
    std::string update(const core::InputState& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;
};

} // namespace scenes
