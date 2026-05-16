#include "Logger.hpp"

#include <SDL3/SDL_filesystem.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <time.h>

#ifdef PONG_DEV
#include <iostream>
#endif

namespace Log {

namespace {

std::ofstream g_log_file;

struct tm get_current_localtime() {
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now()
    );

    struct tm local_time;

#ifdef _WIN32
    localtime_s(&local_time, &now);
#else
    localtime_r(&now, &local_time);
#endif

    return local_time;
}

std::string current_timestamp() {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
              ) % 1000;

    struct tm lt = get_current_localtime();

    return std::format("{:02}:{:02}:{:02}.{:03}",
                       lt.tm_hour, lt.tm_min, lt.tm_sec, ms.count());
}

std::string current_datetime() {
    struct tm lt = get_current_localtime();

    return std::format("{:04}-{:02}-{:02} {}",
                       lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday, current_timestamp());
}

} // namespace

void init(std::string_view version, std::string_view build_mode) {
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
    g_log_file << std::format("Build: {}\n", build_mode);
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
