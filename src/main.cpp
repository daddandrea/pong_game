#include "core/Application.hpp"
#include "core/Logger.hpp"

int main() {

    if (!core::Application::init()) return 1;

    int exit_code = 0;

    try {
        core::Application app;
        app.run();

    } catch (const std::exception& ex) {
        Log::error("Fatal error: {}\n", ex.what());
        core::Application::quit();
        exit_code = 1;
    }

    core::Application::quit();
    return exit_code;
}
