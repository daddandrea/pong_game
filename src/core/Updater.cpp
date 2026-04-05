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
static const std::string ARCHIVE_NAME = PONG_ARCHIVE_WINDOWS + std::string(".zip");
static const std::string SCRIPT_NAME  = "update.bat";
#else
static const std::string ARCHIVE_NAME = PONG_ARCHIVE_LINUX + std::string(".tar.gz");
static const std::string SCRIPT_NAME  = "update.sh";
#endif

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
    if (!res) return res;

    // follow one redirect if we get a 3xx with a Location header
    if (res->status >= 300 && res->status < 400) {
        const auto it = res->headers.find("Location");
        if (it != res->headers.end()) {
            std::string rhost, rpath;
            parse_url(it->second, rhost, rpath);

            httplib::Client rcli(rhost);
            rcli.set_connection_timeout(10);
            rcli.set_read_timeout(60);
            return rcli.Get(rpath, {
                { "User-Agent", "pong-updater" },
                { "Accept",     "*/*"           }
            });
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

    const std::string tag = parse_string_field(res->body, "tag_name");
    if (tag.empty()) {
        Log::warn("Updater: could not parse tag_name");
        m_status = Status::Error;
        return;
    }

    // find the browser_download_url for the platform-specific archive
    const std::string url = [&]() -> std::string {
        const std::string& body = res->body;
        std::string::size_type pos = 0;
        while ((pos = body.find(ARCHIVE_NAME, pos)) != std::string::npos) {
            // search backwards for browser_download_url on the same asset entry
            auto url_pos = body.rfind("browser_download_url", pos);
            if (url_pos != std::string::npos)
                return parse_string_field(body.substr(url_pos), "browser_download_url");
            ++pos;
        }
        return {};
    }();

    if (url.empty()) {
        Log::warn("Updater: could not find download URL for {}", ARCHIVE_NAME);
        m_status = Status::Error;
        return;
    }

    m_latest_version = tag;
    m_download_url   = url;

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
    m_status = Status::Downloading;

    const char* base = SDL_GetBasePath();
    if (!base) {
        Log::error("Updater: SDL_GetBasePath failed");
        m_status = Status::InstallFailed;
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
