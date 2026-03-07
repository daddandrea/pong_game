#pragma once

#include "core/AudioManager.hpp"
#include "core/InputState.hpp"
#include "game/BallState.hpp"
#include "game/CpuController.hpp"
#include "game/GameConfig.hpp"
#include "game/PaddleState.hpp"
#include "scenes/IScene.hpp"

namespace scenes {

enum class GamePhase {
    Countdown,
    Playing,
    PointScored,
    GameOver,
};

enum class Winner { Left, Right };

class GameScene : public IScene {

public:
    GameScene();

    void on_enter() override;
    std::string update(const core::InputState& input, float dt) override;
    void render(renderer::Renderer2D& r) const override;

private:
    void reset_round(bool ball_goes_right);

    game::BallState m_ball;
    game::PaddleState m_left_paddle;
    game::PaddleState m_right_paddle;
    game::CpuController m_cpu;

    game::GameConfig m_config;

    int m_score_right = 0;
    int m_score_left = 0;

    GamePhase m_phase = GamePhase::Countdown;
    float m_timer = 0.0f;
    bool m_ball_goes_right = true;

    Winner m_winner = Winner::Left;

    static constexpr float COUNTDOWN_TIME = 2.0f;
    static constexpr float POINT_FREEZE_TIME = 1.5f;
};

} // namespace scenes
