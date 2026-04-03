#pragma once

#include "game/GameSettings.hpp"
#include "game/GameState.hpp"

namespace scenes {

#ifdef PONG_DEV
void on_win_score_changed(game::GameState* state);
void render_dev_menu(game::GameState* state, game::GameSettings& settings);
#else
inline void render_dev_menu(game::GameState*, game::GameSettings&) {}
#endif

} // namespace scenes
