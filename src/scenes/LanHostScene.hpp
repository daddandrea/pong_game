#pragma once

#include "game/GameConfig.hpp"
#include "scenes/IScene.hpp"
#include <vector>

namespace scenes {

enum class LanHostMenuItem {
    Cancel
};

class LanHostScene : public IScene {

public:
    explicit LanHostScene(game::GameConfig& config);

    std::string update(const core::InputState &input, float dt) override;
    void render(renderer::Renderer2D &r) const override;

private:
    game::GameConfig& m_config;
    std::vector<renderer::Button> m_buttons;

    static constexpr float BTN_W = 5.0f;
    static constexpr float BTN_H = 0.9f;
};

}
