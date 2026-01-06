#pragma once

#include "../utils/types.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace pominal {

class timer {
public:
    using tick_callback = std::function<void(const timer_status&)>;
    using state_change_callback = std::function<void(timer_state, timer_state)>;
    using warning_callback = std::function<void(const timer_status&)>;

    explicit timer(const timer_config& config = {});
    ~timer();

    timer(const timer&) = delete;
    auto operator=(const timer&) -> timer& = delete;
    timer(timer&&) = delete;
    auto operator=(timer&&) -> timer& = delete;

    auto start() -> bool;
    auto pause() -> bool;
    auto stop() -> bool;
    auto reset() -> bool;
    auto skip() -> bool;

    auto set_focus() -> bool;
    auto set_short_break() -> bool;
    auto set_long_break() -> bool;
    auto set_interval(std::uint8_t interval) -> bool;

    [[nodiscard]] auto get_status() const -> timer_status;
    [[nodiscard]] auto get_config() const -> timer_config;

    auto set_tick_callback(tick_callback callback) -> void;
    auto set_state_change_callback(state_change_callback callback) -> void;
    auto set_warning_callback(warning_callback callback) -> void;

    auto update_config(const timer_config& config) -> void;

private:
    auto timer_loop() -> void;
    auto transition_to_next_state() -> void;
    auto calculate_duration(timer_state state) const -> std::chrono::seconds;

    timer_config config_;
    timer_status status_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::thread timer_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> active_{false};

    tick_callback tick_callback_;
    state_change_callback state_change_callback_;
    warning_callback warning_callback_;
};

}
