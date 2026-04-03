#pragma once

#include "core/InputState.hpp"
#include "game/BallState.hpp"
#include "game/CpuController.hpp"
#include "game/GameConfig.hpp"
#include "game/GameState.hpp"
#include "game/PaddleState.hpp"
#include "scenes/IScene.hpp"

namespace scenes {

class GameScene : public IScene {

public:
    GameScene();

    void on_enter() override;
    std::string update(const core::InputState& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;
    game::GameState* get_game_state() override { return &m_game_state; };

private:
    void reset_round(bool ball_goes_right);

    game::BallState m_ball;
    game::PaddleState m_left_paddle;
    game::PaddleState m_right_paddle;
    game::CpuController m_cpu;

    game::GameConfig m_config;

    game::GameState m_game_state;

    float m_timer = 0.0f;
    bool m_ball_goes_right = true;

    static constexpr float COUNTDOWN_TIME = 2.0f;
    static constexpr float POINT_FREEZE_TIME = 1.5f;
};

} // namespace scenes
