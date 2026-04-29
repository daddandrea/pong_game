#pragma once

#include "game/DevSettings.hpp"
#include "game/GameSettings.hpp"
#include "game/GameState.hpp"

namespace scenes {

#ifdef PONG_DEV
void on_win_score_changed(game::GameState* state);
void render_dev_menu(game::GameState* state, game::GameSettings& settings, game::DevSettings& dev);
#else
inline void render_dev_menu(game::GameState*, game::GameSettings&, game::DevSettings&) {}
#endif

} // namespace scenes
