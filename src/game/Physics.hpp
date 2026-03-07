#pragma once

namespace game {

struct BallState;
struct PaddleState;

bool handle_ball_paddle_collision(BallState& ball, PaddleState& paddle, bool parry_active = false);

int check_score(const BallState& ball);

bool is_game_over(int score_left, int score_right, int win_score);

bool handle_wall_bounce(BallState& ball);
}
