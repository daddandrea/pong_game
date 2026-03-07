#pragma once

#include <string>

namespace core { struct InputState; }
namespace renderer { class Renderer2D; }

namespace Transition {

inline constexpr const char* Stay      = "";
inline constexpr const char* Quit      = "quit";
inline constexpr const char* Pop       = "<";

inline constexpr const char* Game      = "game";
inline constexpr const char* MainMenu  = "main_menu";
inline constexpr const char* Pause     = "pause";
inline constexpr const char* Credits   = "credits";

inline std::string Push(const char* scene) {
    return std::string(">") + scene;
}

inline bool is_push(const std::string& t) {
    return t.size() > 1 && t[0] == '>';
}

inline std::string get_push_target(const std::string& t) {
    return t.substr(1);
}

}

namespace scenes {

class IScene {
public:
    virtual ~IScene() = default;

    virtual void on_enter() {}

    virtual void on_resume() {}

    virtual void on_exit() {}

    virtual std::string update(const core::InputState& input, float dt) = 0;

    virtual void render(renderer::Renderer2D& r) const = 0;
};
}
