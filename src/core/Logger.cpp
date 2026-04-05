#include "Logger.hpp"

#include <SDL3/SDL_filesystem.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>

#ifdef PONG_DEV
#include <iostream>
#endif

namespace Log {

namespace {

std::ofstream g_log_file;

std::string current_timestamp() {
    using namespace std::chrono;
    auto now   = system_clock::now();
    auto ms    = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t  = system_clock::to_time_t(now);
    std::tm*   tm  = std::localtime(&t);
    return std::format("{:02}:{:02}:{:02}.{:03}",
                       tm->tm_hour, tm->tm_min, tm->tm_sec, ms.count());
}

std::string current_datetime() {
    using namespace std::chrono;
    auto now  = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm*  tm  = std::localtime(&t);
    return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
}

} // namespace

void init(std::string_view version) {
    const char* base = SDL_GetBasePath();
    if (!base) return;

    std::filesystem::path log_dir = std::filesystem::path(base) / "log";
    std::filesystem::create_directories(log_dir);

    auto log_path  = log_dir / "pong.log";
    auto prev_path = log_dir / "pong_prev.log";

    if (std::filesystem::exists(log_path))
        std::filesystem::rename(log_path, prev_path);

    g_log_file.open(log_path);
    if (!g_log_file.is_open()) return;

    g_log_file << std::format("Pong v{}\n", version);
    g_log_file << std::format("Session started: {}\n", current_datetime());
    g_log_file << std::string(60, '-') << '\n';
    g_log_file.flush();
}

namespace detail {

void print_info(std::string_view msg, const std::source_location& loc) {
    const auto ts = current_timestamp();
    if (g_log_file.is_open()) {
        g_log_file << std::format("[{}] [INFO] {}:{} | {}\n", ts, loc.file_name(), loc.line(), msg);
        g_log_file.flush();
    }
#ifdef PONG_DEV
    std::cout << std::format("\033[34m[{}] [INFO] {}:{} | {}\033[0m\n", ts, loc.file_name(), loc.line(), msg);
#endif
}

void print_warn(std::string_view msg, const std::source_location& loc) {
    const auto ts = current_timestamp();
    if (g_log_file.is_open()) {
        g_log_file << std::format("[{}] [WARN] {}:{} | {}\n", ts, loc.file_name(), loc.line(), msg);
        g_log_file.flush();
    }
#ifdef PONG_DEV
    std::cout << std::format("\033[33m[{}] [WARN] {}:{} | {}\033[0m\n", ts, loc.file_name(), loc.line(), msg);
#endif
}

void print_error(std::string_view msg, const std::source_location& loc) {
    const auto ts = current_timestamp();
    if (g_log_file.is_open()) {
        g_log_file << std::format("[{}] [ERROR] {}:{} | {}\n", ts, loc.file_name(), loc.line(), msg);
        g_log_file.flush();
    }
#ifdef PONG_DEV
    std::cerr << std::format("\033[31m[{}] [ERROR] {}:{} | {}\033[0m\n", ts, loc.file_name(), loc.line(), msg);
#endif
}

} // namespace detail
} // namespace Log
