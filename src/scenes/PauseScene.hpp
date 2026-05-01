#pragma once

#include "game/DevSettings.hpp"
#include "scenes/IScene.hpp"
#include "scenes/MainMenuScene.hpp"
#include "scenes/SceneManager.hpp"

namespace scenes {

enum class PauseItem { Resume, MainMenu, Quit };

class PauseScene : public IScene {

public:
    explicit PauseScene(game::DevSettings& dev_settings);

    std::string update(const core::InputState& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;

private:
    game::DevSettings& m_dev;
    std::vector<Button> m_buttons;

    static constexpr float BTN_W = 4.0f;
    static constexpr float BTN_H = 0.85f;
};

} // namespace scenes
