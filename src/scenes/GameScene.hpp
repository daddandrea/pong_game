#pragma once

#include "game/BallState.hpp"
#include "game/CpuController.hpp"
#include "game/GameConfig.hpp"
#include "game/GameSettings.hpp"
#include "game/GameState.hpp"
#include "game/PaddleState.hpp"
#include "scenes/IScene.hpp"

namespace scenes {

class GameScene : public IScene {

public:
    GameScene(game::GameSettings& settings, game::GameConfig& config);

    void on_enter() override;
    std::string update(const core::FrameInput& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;
    game::GameState* get_game_state() override { return &m_game_state; };

private:
    static constexpr float COUNTDOWN_TIME = 2.0f;
    static constexpr float POINT_FREEZE_TIME = 1.5f;

    game::GameSettings& m_settings;
    game::GameConfig&   m_config;

    game::BallState m_ball;
    game::PaddleState m_left_paddle  = game::PaddleState(game::PaddleSide::Left);
    game::PaddleState m_right_paddle = game::PaddleState(game::PaddleSide::Right);

    [[no_unique_address]] game::CpuController m_cpu;

    game::GameConfig m_local_config;

    game::GameState m_game_state;

    float m_timer = 0.0f;
    bool m_ball_goes_right = true;

    void reset_round(bool ball_goes_right);
    void handle_paddle_movement(const core::FrameInput& input, float dt);
    void handle_ball_physics(const core::FrameInput& input, float dt);
    void handle_scoring();
};

} // namespace scenes
