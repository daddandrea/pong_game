#pragma once

namespace game {

enum class PlayerType { Human, Cpu };

static constexpr int STANDARD_WIN_SCORE = 7;

struct GameConfig {
    PlayerType left  = PlayerType::Human;
    PlayerType right = PlayerType::Cpu;
    int win_score    = STANDARD_WIN_SCORE;
};

inline GameConfig g_config;

}
