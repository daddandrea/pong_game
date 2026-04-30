#include "GameScene.hpp"
#include "core/AudioManager.hpp"
#include "core/InputState.hpp"
#include "game/Math.hpp"
#include "game/Physics.hpp"
#include "renderer/Renderer2D.hpp"
#include "scenes/Colors.hpp"

#include <random>
#include <string>

namespace scenes {

static bool rand_bool() {
    thread_local std::mt19937 gen{std::random_device{}()};
    return std::bernoulli_distribution{0.5}(gen);
}

GameScene::GameScene(game::GameSettings& settings, game::GameConfig& config, game::DevSettings& dev)
    : m_settings(settings),
      m_config(config),
      m_dev(dev)
{}

void GameScene::on_enter() {
    m_local_config = m_config;

    m_game_state.score_left  = 0;
    m_game_state.score_right = 0;
    m_game_state.win_score = m_local_config.win_score;
    m_ball_goes_right = rand_bool();

    m_left_paddle  = game::PaddleState(game::PaddleSide::Left);
    m_right_paddle = game::PaddleState(game::PaddleSide::Right);

    reset_round(m_ball_goes_right);
}

void GameScene::reset_round(bool ball_goes_right) {
    m_ball.reset();
    m_ball_goes_right = ball_goes_right;
    m_game_state.phase = game::GamePhase::Countdown;
    m_timer = COUNTDOWN_TIME;
}

void GameScene::handle_paddle_movement(const core::InputState& input, float dt) {
    using enum core::Key;

    const bool left_up    = input.is_held(W);
    const bool left_down  = input.is_held(S);
    const bool right_down = input.is_held(Down);
    const bool right_up   = input.is_held(Up);

    if (m_local_config.left == game::PlayerType::Human) {
        m_left_paddle.update(dt, left_up, left_down, m_settings);
    } else {
        m_cpu.update(m_left_paddle, m_ball, dt, m_settings);
    }

    if (m_local_config.right == game::PlayerType::Human) {
        m_right_paddle.update(dt, right_up, right_down, m_settings);
    } else {
        m_cpu.update(m_right_paddle, m_ball, dt, m_settings);
    }
}

void GameScene::handle_ball_physics(const core::InputState& input, float dt) {
    const bool left_parry_key  = input.is_pressed(core::Key::Space);
    const bool right_parry_key = input.is_pressed(core::Key::Enter);

    m_ball.update(dt);
    if (game::handle_wall_bounce(m_ball, m_settings))
        core::AudioManager::get().play(core::AudioManager::Sound::WallHit);

    const bool approaching_left  = m_ball.hor_dir < 0.0f;
    const bool approaching_right = m_ball.hor_dir > 0.0f;

    const bool left_parry  = left_parry_key
        && approaching_left
        && m_local_config.left == game::PlayerType::Human;

    const bool right_parry = right_parry_key
        && approaching_right
        && m_local_config.right == game::PlayerType::Human;

    if (game::handle_ball_paddle_collision(m_ball, m_left_paddle, m_settings, left_parry))
        core::AudioManager::get().play(core::AudioManager::Sound::PaddleHit);

    if (game::handle_ball_paddle_collision(m_ball, m_right_paddle, m_settings, right_parry))
        core::AudioManager::get().play(core::AudioManager::Sound::PaddleHit);
}

void GameScene::handle_scoring() {
    const int scored = game::calc_score(m_ball, m_settings);
    if (scored != 0) {
        core::AudioManager::get().play(core::AudioManager::Sound::Score);

        scored > 0 ? m_game_state.score_right++ : m_game_state.score_left++;

        m_ball.reset();
        m_game_state.phase = game::GamePhase::PointScored;
        m_timer = POINT_FREEZE_TIME;

        m_ball_goes_right = (scored > 0);

        if (game::is_game_over(m_game_state.score_left, m_game_state.score_right, m_game_state.win_score)) {
            m_game_state.winner = (m_game_state.score_left >= m_game_state.win_score)
                ? game::Winner::Left
                : game::Winner::Right;
            m_game_state.phase = game::GamePhase::GameOver;
        }
    }
}

std::string GameScene::update(const core::InputState& input, float dt) {
    using namespace core;

    if (input.is_pressed(Key::Escape)) return Transition::Push(Transition::Pause);
#ifdef PONG_DEV
    if (input.is_pressed(Key::F1)) m_dev.show_dev = !m_dev.show_dev;
#endif

    switch (m_game_state.phase) {
        using enum game::GamePhase;

        case Countdown:
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                m_ball.launch(m_ball_goes_right, rand_bool(), m_settings);
                m_game_state.phase = game::GamePhase::Playing;
            }
            break;

        case Playing: {
            handle_paddle_movement(input, dt);
            handle_ball_physics(input, dt);
            handle_scoring();
            break;
        }

        case PointScored:
            m_timer -= dt;
            if (m_timer <= 0.0f)
                reset_round(m_ball_goes_right);

            break;

        case GameOver:
            if (input.is_pressed(Key::Space) || input.is_pressed(Key::Escape))
                return Transition::MainMenu;
            break;
    }

    return Transition::Stay;
}

void GameScene::render(renderer::Renderer2D& r) const {
    using namespace glm;

    // ── Court ─────────────────────────────────────────────────────────
    r.draw_rect_outline({0.0f, 0.0f}, {20.0f, 12.0f}, {0.4f, 0.4f, 0.4f, 1.0f}, game::ARENA_WALL_THICKNESS);

    {
        constexpr int   DASHES = 10;
        constexpr float DASH_H = 0.4f;
        constexpr float GAP_H  = 0.3f;
        constexpr float STEP   = DASH_H + GAP_H;

        const float start_y = -(DASHES / 2.0f) * STEP + DASH_H / 2.0f;

        for (int i = 0; i < DASHES; ++i) {
            r.draw_quad({0.0f, start_y + static_cast<float>(i) * STEP}, 
                        {0.08f, DASH_H}, 
                        {0.4f, 0.4f, 0.4f, 0.8f});

        }
    }

    // ── Paddles ─────────────────────────────────────────────────────────
    r.draw_quad(m_left_paddle.pos,
                {game::PADDLE_HALF_W * 2.0f, m_settings.paddle_half_h * 2.0f},
                Colors::PlayerLeft);

    r.draw_quad(m_right_paddle.pos,
                {game::PADDLE_HALF_W * 2.0f, m_settings.paddle_half_h * 2.0f},
                Colors::PlayerRight);


    // ── Ball ─────────────────────────────────────────────────────────
    if (m_ball.in_play) {
        r.draw_circle(m_ball.pos, m_settings.ball_radius, Colors::BallColor);
    }

    // ── Scores ─────────────────────────────────────────────────────────
    r.draw_text(std::to_string(m_game_state.score_left),  -4.0f, 4.8f, 1.8f, Colors::PlayerLeft);
    r.draw_text(std::to_string(m_game_state.score_right),  4.0f, 4.8f, 1.8f, Colors::PlayerRight);

    // ── Phase overlay ─────────────────────────────────────────────────────────
    switch (m_game_state.phase) {
        case game::GamePhase::Countdown:
            r.draw_text("Get ready!", 0.0f, 0.5f, 1.0f, Colors::MainWhite);
            r.draw_text(std::to_string(static_cast<int>(m_timer) + 1), 0.0f, -0.8f, 2.0f, Colors::MainWhite);
            break;

        case game::GamePhase::PointScored:
            r.draw_text("Point!", 0.0f, 0.0f, 1.2f, Colors::MainWhite);
            break;

        case game::GamePhase::GameOver: {
            const char* winner = (m_game_state.winner == game::Winner::Left)
                                   ? "Left player wins!"
                                   : "Right player wins";

            const glm::vec4 col = (m_game_state.winner == game::Winner::Left)
                                    ? Colors::PlayerLeft
                                    : Colors::PlayerRight;

            r.draw_quad({0.0f, 0.0f}, {14.0f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.75f});
            r.draw_text(winner, 0.0f, 0.8f, 1.1f, col);
            r.draw_text("Press Space to return to menu", 0.0f, -0.8f, 0.6f, Colors::MainSubtle);
            break;
        }

        default:
            break;
    }

    if (!m_left_paddle.parry_on_cooldown())
        r.draw_circle({m_left_paddle.pos.x + 0.6f, m_left_paddle.pos.y}, 0.12f, {0.4f, 1.0f, 0.4f, 0.8f});

    if (!m_right_paddle.parry_on_cooldown())
        r.draw_circle({m_right_paddle.pos.x - 0.6f, m_right_paddle.pos.y}, 0.12f, {0.4f, 1.0f, 0.4f, 0.8f});

}

} // namespace scenes
