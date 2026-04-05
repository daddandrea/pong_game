#include "Updater.hpp"
#include "Logger.hpp"

#include <httplib.h>
#include <SDL3/SDL_filesystem.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include <SDL3/SDL_events.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef PONG_VERSION
#define PONG_VERSION "0.0.0"
#endif
#ifndef PONG_GITHUB_REPO
#define PONG_GITHUB_REPO "daddandrea/pong_game"
#endif
#ifndef PONG_ARCHIVE_LINUX
#define PONG_ARCHIVE_LINUX "pong-linux-x86_64"
#endif
#ifndef PONG_ARCHIVE_WINDOWS
#define PONG_ARCHIVE_WINDOWS "pong-windows-x86_64"
#endif

namespace core {

static const std::string GITHUB_API_HOST = "https://api.github.com";
static const std::string GITHUB_HOST     = "https://github.com";
static const std::string RELEASE_PATH    = "/repos/" PONG_GITHUB_REPO "/releases/latest";

#ifdef _WIN32
static const std::string SCRIPT_NAME = "update.bat";
#else
static const std::string SCRIPT_NAME = "update.sh";
#endif

static std::string versioned_archive(const std::string& tag) {
#ifdef _WIN32
    return std::string(PONG_ARCHIVE_WINDOWS) + "-" + tag + ".zip";
#else
    return std::string(PONG_ARCHIVE_LINUX) + "-" + tag + ".tar.gz";
#endif
}

/**
 * @brief Extracts the value of a string field from a GitHub API JSON response.
 *
 * Only handles the simple case: "key": "value"
 *
 * @param json Raw JSON string.
 * @param key  The field name to look up (without quotes).
 * @return The field value string, or an empty string if not found.
 */
static std::string parse_string_field(const std::string& json, const std::string& key) {
    const std::string quoted_key = "\"" + key + "\"";
    auto pos = json.find(quoted_key);
    if (pos == std::string::npos) return {};
    // skip past the key and colon/space to the opening quote of the value
    pos = json.find('"', pos + quoted_key.size() + 1);
    if (pos == std::string::npos) return {};
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return json.substr(pos + 1, end - pos - 1);
}

/**
 * @brief Parses an ISO 8601 UTC timestamp string to a Unix timestamp.
 *
 * Expected format: "2026-04-05T14:32:01Z"
 *
 * @param s ISO 8601 string.
 * @return Unix timestamp (seconds since epoch), or 0 on parse failure.
 */
static int64_t parse_iso8601(const std::string& s) {
    if (s.size() < 20) return 0;
    try {
        std::tm tm{};
        tm.tm_year = std::stoi(s.substr(0,  4)) - 1900;
        tm.tm_mon  = std::stoi(s.substr(5,  2)) - 1;
        tm.tm_mday = std::stoi(s.substr(8,  2));
        tm.tm_hour = std::stoi(s.substr(11, 2));
        tm.tm_min  = std::stoi(s.substr(14, 2));
        tm.tm_sec  = std::stoi(s.substr(17, 2));
#ifdef _WIN32
        return static_cast<int64_t>(_mkgmtime(&tm));
#else
        return static_cast<int64_t>(timegm(&tm));
#endif
    } catch (...) {
        return 0;
    }
}

/**
 * @brief Parses the host (with scheme) and path from a URL string.
 *
 * e.g. "https://example.com/foo/bar" -> host="https://example.com", path="/foo/bar"
 *
 * @param url   Full URL string.
 * @param host  Output: scheme + host, e.g. "https://example.com".
 * @param path  Output: path component, e.g. "/foo/bar".
 */
static void parse_url(const std::string& url, std::string& host, std::string& path) {
    const std::string prefix = "https://";
    const auto host_start    = url.substr(0, prefix.size()) == prefix ? prefix.size() : 0;
    const auto path_start    = url.find('/', host_start);
    host = prefix + url.substr(host_start, path_start - host_start);
    path = url.substr(path_start);
}

/**
 * @brief Performs a GET request and manually follows one redirect if needed.
 *
 * cpp-httplib does not reliably follow cross-domain HTTPS redirects.
 * GitHub release asset URLs redirect from github.com to a CDN
 * (objects.githubusercontent.com), so we handle the redirect explicitly.
 *
 * @param url Full URL to fetch.
 * @return The final HTTP response, or nullptr on failure.
 */
static httplib::Result fetch_with_redirect(const std::string& url) {
    std::string host, path;
    parse_url(url, host, path);

    httplib::Client cli(host);
    cli.set_follow_location(false);
    cli.set_connection_timeout(10);
    cli.set_read_timeout(60);

    auto res = cli.Get(path, {{ "User-Agent", "pong-updater" }});
    if (!res) {
        Log::warn("Updater: initial request failed (no response)");
        return res;
    }

    Log::info("Updater: initial response status {}", res->status);

    // follow one redirect if we get a 3xx with a Location header
    if (res->status >= 300 && res->status < 400) {
        const auto it = res->headers.find("Location");
        if (it != res->headers.end()) {
            Log::info("Updater: redirecting to {}", it->second);
            std::string rhost, rpath;
            parse_url(it->second, rhost, rpath);

            httplib::Client rcli(rhost);
            rcli.set_connection_timeout(10);
            rcli.set_read_timeout(60);
            auto rres = rcli.Get(rpath, {
                { "User-Agent", "pong-updater" },
                { "Accept",     "*/*"           }
            });
            if (rres) Log::info("Updater: redirect response status {}", rres->status);
            else      Log::warn("Updater: redirect request failed (no response)");
            return rres;
        } else {
            Log::warn("Updater: got {} but no Location header", res->status);
        }
    }

    return res;
}

/**
 * @brief Launches the update script with the given arguments.
 *
 * On Linux: uses fork + exec to avoid shell interpretation.
 * On Windows: uses CreateProcess for the same reason, though a command
 * line string is still required by the Win32 API.
 *
 * @param script      Path to the update script (.sh or .bat).
 * @param archive     Path to the downloaded archive.
 * @param install_dir Directory where the game is installed.
 * @param executable  Path to the game binary to relaunch after update.
 */
static void launch_script(
    const std::filesystem::path& script,
    const std::filesystem::path& archive,
    const std::filesystem::path& install_dir,
    const std::filesystem::path& executable)
{
#ifdef _WIN32
    std::string cmd = std::format("\"{}\" \"{}\" \"{}\" \"{}\"",
        script.string(), archive.string(), install_dir.string(), executable.string());

    STARTUPINFOA si = {};
    PROCESS_INFORMATION pi = {};
    si.cb = sizeof(si);
    CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE,
                   CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    std::filesystem::permissions(script,
        std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::add);

    pid_t pid = fork();
    if (pid == 0) {
        setsid(); // detach from the parent's process group and terminal

        // redirect stdin/stdout/stderr to /dev/null so the child
        // and anything it launches don't inherit the terminal
        int devnull = open("/dev/null", O_RDWR);
        if (devnull >= 0) {
            dup2(devnull, STDIN_FILENO);
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        execl("/bin/sh", "sh",
            script.c_str(),
            archive.c_str(),
            install_dir.c_str(),
            executable.c_str(),
            nullptr);
        _exit(1);
    }
#endif
}


Updater::Updater()  = default;
Updater::~Updater() {
    if (m_thread.joinable())
        m_thread.join();
}

void Updater::check_async() {
    m_status = Status::Checking;
    m_thread = std::thread(&Updater::do_check, this);
}

Updater::Status Updater::status() const {
    return m_status.load();
}

std::string Updater::latest_version() const {
    return m_latest_version;
}

static bool has_git_folder() {
    const char* base = SDL_GetBasePath();
    if (!base) return false;

    std::filesystem::path dir = base;
    for (int i = 0; i < 4; ++i) {
        if (std::filesystem::exists(dir / ".git"))
            return true;
        const auto parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }
    return false;
}

void Updater::do_check() {
    if (has_git_folder()) {
        Log::info("Updater: .git folder found, skipping update check (use git pull)");
        m_status = Status::DevBuild;
        return;
    }

    httplib::Client cli(GITHUB_API_HOST);
    cli.set_follow_location(true);
    cli.set_connection_timeout(5);
    cli.set_read_timeout(5);

    auto res = cli.Get(RELEASE_PATH, {
        { "User-Agent", "pong-updater" },
        { "Accept",     "application/vnd.github+json" }
    });

    if (!res || res->status != 200) {
        Log::warn("Updater: failed to reach GitHub API");
        m_status = Status::Error;
        return;
    }

    const std::string tag = parse_string_field(res->body, "tag_name");
    if (tag.empty()) {
        Log::warn("Updater: could not parse tag_name");
        m_status = Status::Error;
        return;
    }

    const std::string published_at = parse_string_field(res->body, "published_at");
    const int64_t release_time = parse_iso8601(published_at);
    if (release_time == 0) {
        Log::warn("Updater: could not parse published_at");
        m_status = Status::Error;
        return;
    }

    m_latest_version = tag;
    m_download_url   = std::format("https://github.com/{}/releases/download/{}/{}",
                                   PONG_GITHUB_REPO, tag, versioned_archive(tag));

    if (release_time > static_cast<int64_t>(PONG_BUILD_TIME)) {
        Log::info("Updater: update available (released {}, built {})", release_time, PONG_BUILD_TIME);
        m_status = Status::UpdateAvailable;
    } else {
        Log::info("Updater: up to date ({})", PONG_VERSION);
        m_status = Status::UpToDate;
    }
}

void Updater::dismiss() {
    m_status = Status::Dismissed;
}

void Updater::download_and_install() {
    if (m_thread.joinable())
        m_thread.join();
    m_thread = std::thread(&Updater::do_install, this);
}

void Updater::do_install() {
    m_status = Status::Downloading;

    const char* base = SDL_GetBasePath();
    if (!base) {
        Log::error("Updater: SDL_GetBasePath failed");
        m_status = Status::InstallFailed;
        return;
    }

    const std::filesystem::path install_dir    = base;
    const auto                  last_slash     = m_download_url.rfind('/');
    const std::filesystem::path archive        = install_dir / m_download_url.substr(last_slash + 1);
    const std::filesystem::path script         = install_dir / SCRIPT_NAME;
#ifdef _WIN32
    const std::filesystem::path executable  = install_dir / "Pong.exe";
#else
    const std::filesystem::path executable  = install_dir / "Pong";
#endif

    Log::info("Updater: downloading {}", m_download_url);

    auto res = fetch_with_redirect(m_download_url);

    if (!res || res->status != 200) {
        Log::error("Updater: download failed (status {})", res ? res->status : 0);
        m_status = Status::InstallFailed;
        return;
    }

    {
        std::ofstream f(archive, std::ios::binary);
        f.write(res->body.data(), static_cast<std::streamsize>(res->body.size()));
    }

    Log::info("Updater: saved archive to {}", archive.string());

    launch_script(script, archive, install_dir, executable);

    SDL_Event quit;
    quit.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit);
}

} // namespace core
