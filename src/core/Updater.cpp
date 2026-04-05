#include "Updater.hpp"
#include "Logger.hpp"

#include <httplib.h>
#include <SDL3/SDL_filesystem.h>

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
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
static const std::string ARCHIVE_NAME = PONG_ARCHIVE_WINDOWS + std::string(".zip");
static const std::string SCRIPT_NAME  = "update.bat";
#else
static const std::string ARCHIVE_NAME = PONG_ARCHIVE_LINUX + std::string(".tar.gz");
static const std::string SCRIPT_NAME  = "update.sh";
#endif

/**
 * @brief Extracts the value of "tag_name" from a GitHub API JSON response.
 *
 * Only handles the simple case: "tag_name": "1.2.3"
 *
 * @param json Raw JSON string from the GitHub releases API.
 * @return The tag name string, or an empty string if not found.
 */
static std::string parse_tag(const std::string& json) {
    const std::string key = "\"tag_name\"";
    auto pos = json.find(key);
    if (pos == std::string::npos) return {};
    // skip past the key and colon/space to the opening quote of the value
    pos = json.find('"', pos + key.size() + 1);
    if (pos == std::string::npos) return {};
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return json.substr(pos + 1, end - pos - 1);
}

/**
 * @brief Strips a leading 'v' or 'V' from a version string.
 *
 * @param v Version string, e.g. "v1.2.3" or "1.2.3".
 * @return Version string without the leading 'v', e.g. "1.2.3".
 */
static std::string strip_v(const std::string& v) {
    if (!v.empty() && (v[0] == 'v' || v[0] == 'V'))
        return v.substr(1);
    return v;
}

/**
 * @brief Returns the Nth component (0-indexed) of a "X.Y.Z" version string.
 *
 * e.g. version_part("1.2.3", 1) -> 2
 *
 * @param v     Version string in "X.Y.Z" format.
 * @param part  0 = major, 1 = minor, 2 = patch.
 * @return The numeric value of the requested part, or 0 if missing or invalid.
 */
static int version_part(const std::string& v, int part) {
    int i = 0;
    std::string::size_type pos = 0;
    while (i < part) {
        pos = v.find('.', pos);
        if (pos == std::string::npos) return 0;
        ++pos;
        ++i;
    }
    try {
        return std::stoi(v.substr(pos));
    } catch (...) {
        return 0;
    }
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
                   CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    std::filesystem::permissions(script,
        std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::add);

    pid_t pid = fork();
    if (pid == 0) {
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

/**
 * @brief Returns true if @p remote is strictly newer than @p local.
 *
 * Compares major, minor, and patch in order. Both strings may
 * optionally have a leading 'v'.
 *
 * @param remote Version string from the remote release.
 * @param local  Version string of the running binary.
 * @return true if remote > local, false otherwise.
 */
static bool is_newer(const std::string& remote, const std::string& local) {
    const std::string r = strip_v(remote);
    const std::string l = strip_v(local);
    for (int i = 0; i < 3; ++i) {
        int rv = version_part(r, i);
        int lv = version_part(l, i);
        if (rv > lv) return true;
        if (rv < lv) return false;
    }
    return false;
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

void Updater::do_check() {
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

    const std::string tag = parse_tag(res->body);
    if (tag.empty()) {
        Log::warn("Updater: could not parse tag_name");
        m_status = Status::Error;
        return;
    }

    m_latest_version = tag;

    if (is_newer(tag, PONG_VERSION)) {
        Log::info("Updater: update available {} -> {}", PONG_VERSION, tag);
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
    const char* base = SDL_GetBasePath();
    if (!base) {
        Log::error("Updater: SDL_GetBasePath failed");
        return;
    }

    const std::filesystem::path install_dir = base;
    const std::filesystem::path archive     = install_dir / ARCHIVE_NAME;
    const std::filesystem::path script      = install_dir / SCRIPT_NAME;
#ifdef _WIN32
    const std::filesystem::path executable  = install_dir / "Pong.exe";
#else
    const std::filesystem::path executable  = install_dir / "Pong";
#endif

    const std::string download_path = std::format(
        "/{}/releases/download/{}/{}", PONG_GITHUB_REPO, m_latest_version, ARCHIVE_NAME);

    Log::info("Updater: downloading {}", download_path);

    httplib::Client cli(GITHUB_HOST);
    cli.set_follow_location(true);
    cli.set_connection_timeout(10);
    cli.set_read_timeout(60);

    auto res = cli.Get(download_path, {{ "User-Agent", "pong-updater" }});

    if (!res || res->status != 200) {
        Log::error("Updater: download failed (status {})", res ? res->status : 0);
        return;
    }

    {
        std::ofstream f(archive, std::ios::binary);
        f.write(res->body.data(), static_cast<std::streamsize>(res->body.size()));
    }

    Log::info("Updater: saved archive to {}", archive.string());

    launch_script(script, archive, install_dir, executable);
    std::exit(0);
}

} // namespace core
