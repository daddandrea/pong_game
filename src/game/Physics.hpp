#pragma once

namespace game {

struct BallState;
struct PaddleState;
struct GameSettings;

bool handle_ball_paddle_collision(BallState& ball, PaddleState& paddle, const GameSettings& settings, bool parry_active = false);

int calc_score(const BallState& ball, const GameSettings& settings);

bool is_game_over(int score_left, int score_right, int win_score);

bool handle_wall_bounce(BallState& ball, const GameSettings& settings);
}
