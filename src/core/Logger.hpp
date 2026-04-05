#pragma once

#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

namespace Log {

namespace detail {

    inline void print_info(std::string_view msg, const std::source_location& loc) {
        std::cout << std::format("\033[34m[INFO] {}:{} | {}\033[0m\n", loc.file_name(), loc.line(), msg);
    }

    inline void print_warn(std::string_view msg, const std::source_location& loc) {
        std::cout << std::format("\033[33m[WARN] {}:{} | {}\033[0m\n", loc.file_name(), loc.line(), msg);
    }

    inline void print_error(std::string_view msg, const std::source_location& loc) {
        std::cerr << std::format("\033[31m[ERROR] {}:{} | {}\033[0m\n", loc.file_name(), loc.line(), msg);
    }

} // namespace detail

template<typename... Args>
struct info {
    info(std::format_string<Args...> fmt,
         Args&&... args,
         std::source_location loc = std::source_location::current()) {

        detail::print_info(std::format(fmt, std::forward<Args>(args)...),
                           loc);
    }
};

template<typename... Args>
info(std::format_string<Args...>, Args&&...) -> info<Args...>;

template<typename... Args>
struct warn {
    warn(std::format_string<Args...> fmt,
         Args&&... args,
         std::source_location loc = std::source_location::current()) {

        detail::print_warn(std::format(fmt, std::forward<Args>(args)...),
                           loc);
    }
};

template<typename... Args>
warn(std::format_string<Args...>, Args&&...) -> warn<Args...>;

template<typename... Args>
struct error {
    error(std::format_string<Args...> fmt,
          Args&&... args,
          std::source_location loc = std::source_location::current()) {

        detail::print_error(std::format(fmt, std::forward<Args>(args)...),
                           loc);
    }
};

template<typename... Args>
error(std::format_string<Args...>, Args&&...) -> error<Args...>;

} // namespace Log
