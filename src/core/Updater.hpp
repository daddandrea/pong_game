#pragma once

#include <atomic>
#include <string>
#include <thread>

namespace core {

class Updater {
public:
    enum class Status { Idle, Checking, UpdateAvailable, UpToDate, Dismissed, Error };

    Updater();
    ~Updater();

    Updater(const Updater&)            = delete;
    Updater& operator=(const Updater&) = delete;

    void        check_async();
    void        download_and_install();
    void        dismiss();

    Status      status() const;
    std::string latest_version() const;

private:
    void do_check();
    void do_install();

    std::atomic<Status> m_status { Status::Idle };
    std::string         m_latest_version;
    std::thread         m_thread;
};

} // namespace core
