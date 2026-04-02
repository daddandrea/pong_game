#pragma once

#include "core/InputState.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace core { struct InputState; }
namespace renderer { class Renderer2D; }

namespace scenes {

class IScene;

class SceneManager {
public:
    using Factory = std::function<std::unique_ptr<IScene>()>;

    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    void register_scene(const std::string& name, Factory f);

    void push(const std::string& name);

    std::string update(const core::InputState& input, float dt);

    void render(renderer::Renderer2D& r) const;

private:
    std::map<std::string, Factory> m_factories;
    std::vector<std::unique_ptr<IScene>> m_stack;

    void do_push_scene(const std::string& name);
    void do_pop_scene();
    void clear_stack();
};

}
