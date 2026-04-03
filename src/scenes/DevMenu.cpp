#include "DevMenu.hpp"
#include "game/DevSettings.hpp"
#include "game/Physics.hpp"
#include <imgui.h>

namespace scenes {

void on_win_score_changed(game::GameState* state) {
    if (!state || state->phase == game::GamePhase::GameOver) return;
    if (game::is_game_over(state->score_left, state->score_right, state->win_score)
        && state->score_left != state->score_right) {
        state->winner = state->score_left > state->score_right
                        ? game::Winner::Left
                        : game::Winner::Right;
        state->phase = game::GamePhase::GameOver;
    }
}

void render_dev_menu(game::GameState* state, game::GameSettings& settings) {
    if (!game::g_dev.show_dev)
        return;

    ImGui::Begin("Dev Settings");
    ImGui::SliderFloat("Ball base speed", &settings.ball_base_speed, 1.0f, 30.0f);
    ImGui::SliderFloat("Ball speed step", &settings.ball_speed_step, 0.0f, 2.0f);
    ImGui::SliderFloat("Ball parry mult", &settings.ball_parry_mult, 1.0f, 5.0f);
    ImGui::SliderFloat("Ball radius", &settings.ball_radius, 0.05f, 1.0f);
    ImGui::Separator();
    ImGui::SliderFloat("Paddle speed", &settings.paddle_move_speed, 1.0f, 20.0f);
    ImGui::SliderFloat("Paddle height", &settings.paddle_half_h, 0.3f, 4.0f);
    ImGui::SliderFloat("Parry cooldown", &settings.paddle_parry_cd, 0.1f, 3.0f);
    ImGui::Separator();
    ImGui::SliderFloat("CPU speed", &settings.cpu_speed, 1.0f, 20.0f);
    ImGui::Separator();
    if (state && ImGui::SliderInt("Win score", &state->win_score, 1, 21))
        on_win_score_changed(state);
    ImGui::Separator();
    if (ImGui::Button("Reset defaults"))
        settings = game::GameSettings{};
    ImGui::End();
}

} // namespace scenes
