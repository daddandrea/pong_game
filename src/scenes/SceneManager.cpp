#include "SceneManager.hpp"
#include "IScene.hpp"
#include "core/InputState.hpp"
#include <stdexcept>

namespace scenes {

void SceneManager::pop() {
    do_pop_scene();
}

game::GameState* SceneManager::find_game_state() const {
    for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
        if (auto* s = (*it)->get_game_state())
            return s;
    return nullptr;
}

void SceneManager::register_scene(const std::string& name, Factory f) {
    m_factories[name] = std::move(f);
}

void SceneManager::push(const std::string& name) {
    do_push_scene(name);
}

void SceneManager::do_push_scene(const std::string& name) {
    auto it = m_factories.find(name);
    if (it == m_factories.end()) {
        throw std::runtime_error("SceneManager: unknown scene '" + name + "'");
    }

    auto scene = it->second();
    scene->on_enter();
    m_stack.push_back(std::move(scene));
}

void SceneManager::do_pop_scene() {
    if (m_stack.empty()) return;

    m_stack.back()->on_exit();
    m_stack.pop_back();

    if (!m_stack.empty()) m_stack.back()->on_resume();
}

void SceneManager::clear_stack() {
    while (!m_stack.empty()) {
        m_stack.back()->on_exit();
        m_stack.pop_back();
    }
}

std::string SceneManager::update(const core::InputState& input, float dt) {
    if (m_stack.empty()) return Transition::Quit;

    const std::string transition = m_stack.back()->update(input, dt);

    if (transition.empty()) return Transition::Stay;
    if (transition == Transition::Quit) return Transition::Quit;

    if (transition == Transition::Pop) {
        do_pop_scene();
        if (m_stack.empty()) return Transition::Quit;
        return Transition::Stay;
    }

    if (Transition::is_push(transition)) {
        m_stack.back()->on_exit();
        do_push_scene(Transition::get_push_target(transition));
        return Transition::Stay;
    }

    clear_stack();
    do_push_scene(transition);
    return Transition::Stay;
}

void SceneManager::render(renderer::Renderer2D& r) const {
    for (const auto& scene : m_stack) {
        scene->render(r);
    }
}
}
