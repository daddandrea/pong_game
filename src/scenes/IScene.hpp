#pragma once

#include "core/FrameInput.hpp"
#include "game/GameState.hpp"

#include <string>

namespace renderer {
    struct Button;
    class Renderer2D;
}

namespace Transition {

inline constexpr const char* Stay         = "";
inline constexpr const char* Quit         = "quit";
inline constexpr const char* Pop          = "<";

inline constexpr const char* Game         = "game";
inline constexpr const char* MainMenu     = "main_menu";
inline constexpr const char* SinglePlayer = "single_player";
inline constexpr const char* MultiPlayer  = "multi_player";

inline constexpr const char* LanMenu      = "lan_menu";
inline constexpr const char* LanHost      = "lan_host";
inline constexpr const char* LanJoin      = "lan_join";
inline constexpr const char* MatchMaking  = "match_making";

inline constexpr const char* Pause        = "pause";
inline constexpr const char* Credits      = "credits";

inline std::string Push(const char* scene) {
    return std::string(">") + scene;
}

inline bool is_push(const std::string& t) {
    return t.size() > 1 && t[0] == '>';
}

inline std::string get_push_target(std::string_view target) {
    return static_cast<std::string>(target.substr(1));
}

}

namespace scenes {

class IScene {
public:
    virtual ~IScene() = default;

    virtual void on_enter() {
        /* called once when this scene is pushed and becomes the active scene */
    }

    virtual void on_resume() {
        /* called when the scene pushed on top of this one is popped, making this scene active again */
    }

    virtual void on_exit() {
        /* called once when this scene is popped off the stack and destroyed */
    }

    virtual std::string update(const core::FrameInput& input, float dt) = 0;

    virtual void render(renderer::Renderer2D& r) const = 0;

    virtual game::GameState* get_game_state() { return nullptr; }
};
}
