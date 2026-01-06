#pragma once

#include "../timer/timer.hpp"
#include <atomic>

namespace pominal::daemon {

inline constexpr auto pid_file_path{"/tmp/pominal.pid"};

class service {
public:
    explicit service(const timer_config& config = {});

    auto run() -> int;
    auto stop() -> void;

    [[nodiscard]] static auto is_running() -> bool;
    [[nodiscard]] static auto get_running_pid() -> pid_t;

private:
    auto setup_callbacks() -> void;
    auto handle_client(int client_fd) -> void;
    static auto write_pid_file() -> void;
    static auto remove_pid_file() -> void;

    timer timer_;
    std::atomic<bool> running_;
    bool enable_sound_{true};
};

}
