#include "Updater.hpp"
#include "Logger.hpp"

#include <httplib.h>
#include <SDL3/SDL_filesystem.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include <SDL3/SDL_events.h>

#ifdef _WIN32
#include <windows.h>
#include "WinHttpFetch.hpp"
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

static const std::string GITHUB_HOST = "https://github.com";

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

struct ReleaseEntry { std::string title; std::string updated; };

static std::vector<ReleaseEntry> parse_atom_feed(const std::string& xml) {
    std::vector<ReleaseEntry> entries;
    std::string::size_type pos = 0;

    while (true) {
        const auto entry_start = xml.find("<entry>", pos);
        if (entry_start == std::string::npos) break;
        const auto entry_end = xml.find("</entry>", entry_start);
        if (entry_end == std::string::npos) break;

        const std::string block = xml.substr(entry_start, entry_end - entry_start);

        const auto extract = [&](const char* tag) -> std::string {
            const std::string open  = std::string("<")  + tag + ">";
            const std::string close = std::string("</") + tag + ">";
            const auto s = block.find(open);
            if (s == std::string::npos) return {};
            const auto vs = s + open.size();
            const auto e  = block.find(close, vs);
            if (e == std::string::npos) return {};
            return block.substr(vs, e - vs);
        };

        entries.push_back({ extract("title"), extract("updated") });
        pos = entry_end + 8;
    }

    return entries;
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
static void parse_url(std::string_view url, std::string& host, std::string& path) {
    const std::string prefix = "https://";
    const auto host_start    = url.starts_with(prefix) ? prefix.size() : 0;
    const auto path_start    = url.find('/', host_start);

    host = std::format("{}{}",
                       prefix,
                       url.substr(host_start, path_start - host_start)
           );
    path = url.substr(path_start);
}

#ifndef _WIN32
static httplib::Result fetch_with_redirect(const std::string& url) {
    std::string host;
    std::string path;

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

            std::string rhost;
            std::string rpath;
            parse_url(it->second, rhost, rpath);

            httplib::Client rcli(rhost);
            rcli.set_connection_timeout(10);
            rcli.set_read_timeout(60);
            rcli.set_url_encode(false);
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
#endif

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

        if (int devnull = open("/dev/null", O_RDWR); devnull >= 0) {
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
    m_thread = std::jthread(&Updater::do_check, this);
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
try {
    if (has_git_folder()) {
        Log::info("Updater: .git folder found, skipping update check (use git pull)");
        m_status = Status::DevBuild;
        return;
    }

#ifdef _WIN32
    const std::string atom_url = std::string(GITHUB_HOST) + "/" PONG_GITHUB_REPO "/releases.atom";
    auto bytes = winhttp_download(atom_url);
    if (bytes.empty()) {
        Log::warn("Updater: failed to fetch atom feed");
        m_status = Status::Error;
        return;
    }
    const std::string body(bytes.begin(), bytes.end());
#else
    httplib::Client cli(GITHUB_HOST);
    cli.set_follow_location(true);
    cli.set_connection_timeout(5);
    cli.set_read_timeout(5);

    auto res = cli.Get("/" PONG_GITHUB_REPO "/releases.atom", {{ "User-Agent", "pong-updater" }});
    if (!res || res->status != 200) {
        Log::warn("Updater: failed to fetch atom feed (status {})", res ? res->status : 0);
        m_status = Status::Error;
        return;
    }
    const std::string& body = res->body;
#endif

    const auto entries = parse_atom_feed(body);
    if (entries.empty()) {
        Log::warn("Updater: no entries in atom feed");
        m_status = Status::Error;
        return;
    }

    const auto& latest = *std::max_element(entries.begin(), entries.end(),
        [](const ReleaseEntry& a, const ReleaseEntry& b) { return a.updated < b.updated; });
    m_latest_version = latest.title;
    m_download_url   = std::format("https://github.com/{}/releases/download/{}/{}",
                                   PONG_GITHUB_REPO, latest.title, versioned_archive(latest.title));

    if (latest.title == PONG_VERSION) {
        Log::info("Updater: up to date ({})", PONG_VERSION);
        m_status = Status::UpToDate;
        return;
    }

    std::string current_updated;
    for (const auto& e : entries) {
        if (e.title == PONG_VERSION) {
            current_updated = e.updated;
            break;
        }
    }

    if (current_updated.empty() || latest.updated > current_updated) {
        Log::info("Updater: update available {} -> {}", PONG_VERSION, latest.title);
        m_status = Status::UpdateAvailable;
    } else {
        Log::info("Updater: up to date ({})", PONG_VERSION);
        m_status = Status::UpToDate;
    }
} catch (const std::exception& e) {
    Log::error("Updater: do_check exception: {}", e.what());
    m_status = Status::Error;
} catch (...) {
    Log::error("Updater: do_check unknown exception");
    m_status = Status::Error;
}
}

void Updater::dismiss() {
    m_status = Status::Dismissed;
}

void Updater::download_and_install() {
    m_thread = std::jthread(&Updater::do_install, this);
}

void Updater::do_install() {
try {
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

#ifdef _WIN32
    auto bytes = winhttp_download(m_download_url);
    if (bytes.empty()) {
        Log::error("Updater: download failed");
        m_status = Status::InstallFailed;
        return;
    }
    {
        std::ofstream f(archive, std::ios::binary);
        f.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    }
#else
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
#endif

    Log::info("Updater: saved archive to {}", archive.string());

    launch_script(script, archive, install_dir, executable);

    SDL_Event quit;
    quit.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit);
} catch (const std::exception& e) {
    Log::error("Updater: do_install exception: {}", e.what());
    m_status = Status::InstallFailed;
} catch (...) {
    Log::error("Updater: do_install unknown exception");
    m_status = Status::InstallFailed;
}
}

} // namespace core
