#include "timer.hpp"

namespace pominal {

timer::timer(const timer_config& config)
    : config_{config}
      , running_{true}
{
    status_.total_intervals = config_.intervals_before_long_break;
    timer_thread_ = std::thread{&timer::timer_loop, this};
}

timer::~timer() {
    {
        std::lock_guard lock{mutex_};
        running_ = false;
    }
    cv_.notify_all();
    if (timer_thread_.joinable()) {
        timer_thread_.join();
    }
}

auto timer::start() -> bool {
    std::lock_guard lock{mutex_};
    if (status_.state == timer_state::paused) {
        status_.state = status_.previous_state;
        active_ = true;
        cv_.notify_all();
        return true;
    }
    if (status_.state == timer_state::idle) {
        status_.state = timer_state::focus;
        status_.remaining = calculate_duration(timer_state::focus);
        status_.total = status_.remaining;
        status_.one_minute_warning_sent = false;
        active_ = true;
        if (state_change_callback_) {
            state_change_callback_(timer_state::idle, timer_state::focus);
        }
        cv_.notify_all();
        return true;
    }
    return false;
}

auto timer::pause() -> bool {
    std::lock_guard lock{mutex_};
    if (status_.state != timer_state::idle && status_.state != timer_state::paused) {
        status_.previous_state = status_.state;
        status_.state = timer_state::paused;
        active_ = false;
        return true;
    }
    return false;
}

auto timer::stop() -> bool {
    std::lock_guard lock{mutex_};
    const auto old_state = status_.state;
    status_.state = timer_state::idle;
    status_.remaining = std::chrono::seconds{0};
    status_.total = std::chrono::seconds{0};
    status_.current_interval = 1;
    status_.one_minute_warning_sent = false;
    active_ = false;
    if (old_state != timer_state::idle && state_change_callback_) {
        state_change_callback_(old_state, timer_state::idle);
    }
    return true;
}

auto timer::reset() -> bool {
    std::lock_guard lock{mutex_};
    if (status_.state != timer_state::idle) {
        status_.remaining = status_.total;
        status_.one_minute_warning_sent = false;
        return true;
    }
    return false;
}

auto timer::skip() -> bool {
    std::lock_guard lock{mutex_};
    if (status_.state != timer_state::idle && status_.state != timer_state::paused) {
        transition_to_next_state();
        return true;
    }
    return false;
}

auto timer::set_focus() -> bool {
    std::lock_guard lock{mutex_};
    const auto old_state = status_.state;
    status_.state = timer_state::focus;
    status_.remaining = calculate_duration(timer_state::focus);
    status_.total = status_.remaining;
    status_.one_minute_warning_sent = false;
    active_ = true;
    if (state_change_callback_) {
        state_change_callback_(old_state, timer_state::focus);
    }
    cv_.notify_all();
    return true;
}

auto timer::set_short_break() -> bool {
    std::lock_guard lock{mutex_};
    const auto old_state = status_.state;
    status_.state = timer_state::short_break;
    status_.remaining = calculate_duration(timer_state::short_break);
    status_.total = status_.remaining;
    status_.one_minute_warning_sent = false;
    active_ = true;
    if (state_change_callback_) {
        state_change_callback_(old_state, timer_state::short_break);
    }
    cv_.notify_all();
    return true;
}

auto timer::set_long_break() -> bool {
    std::lock_guard lock{mutex_};
    const auto old_state = status_.state;
    status_.state = timer_state::long_break;
    status_.remaining = calculate_duration(timer_state::long_break);
    status_.total = status_.remaining;
    status_.one_minute_warning_sent = false;
    active_ = true;
    if (state_change_callback_) {
        state_change_callback_(old_state, timer_state::long_break);
    }
    cv_.notify_all();
    return true;
}

auto timer::set_interval(const std::uint8_t interval) -> bool {
    std::lock_guard lock{mutex_};
    if (interval >= 1 && interval <= status_.total_intervals) {
        status_.current_interval = interval;
        return true;
    }
    return false;
}

auto timer::get_status() const -> timer_status {
    std::lock_guard lock{mutex_};
    return status_;
}

auto timer::get_config() const -> timer_config {
    std::lock_guard lock{mutex_};
    return config_;
}

auto timer::set_tick_callback(tick_callback callback) -> void {
    std::lock_guard lock{mutex_};
    tick_callback_ = std::move(callback);
}

auto timer::set_state_change_callback(state_change_callback callback) -> void {
    std::lock_guard lock{mutex_};
    state_change_callback_ = std::move(callback);
}

auto timer::set_warning_callback(warning_callback callback) -> void {
    std::lock_guard lock{mutex_};
    warning_callback_ = std::move(callback);
}

auto timer::update_config(const timer_config& config) -> void {
    std::lock_guard lock{mutex_};
    config_ = config;
    status_.total_intervals = config_.intervals_before_long_break;
}

auto timer::timer_loop() -> void {
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    auto last_tick{clock::now()};

    while (running_) {
        std::unique_lock lock{mutex_};

        if (!active_) {
            cv_.wait(lock, [this] {
                return !running_ || active_;
            });
            last_tick = clock::now();
            if (!running_) {
                break;
            }
            continue;
        }

        const auto now{clock::now()};

        if (const auto elapsed{std::chrono::duration_cast<std::chrono::seconds>(now - last_tick)}; elapsed >= 1s) {
            last_tick = now;
            if (status_.remaining > 0s) {
                status_.remaining -= 1s;

                if (!status_.one_minute_warning_sent && status_.remaining <= 60s && status_.remaining > 0s) {
                    status_.one_minute_warning_sent = true;
                    if (warning_callback_) {
                        const auto callback{warning_callback_};
                        const auto status_copy{status_};
                        lock.unlock();
                        callback(status_copy);
                        lock.lock();
                    }
                }

                if (tick_callback_) {
                    const auto callback{tick_callback_};
                    const auto status_copy{status_};
                    lock.unlock();
                    callback(status_copy);
                    lock.lock();
                }
            }

            if (status_.remaining <= 0s && status_.state != timer_state::idle) {
                transition_to_next_state();
            }
        }

        lock.unlock();
        std::this_thread::sleep_for(100ms);
    }
}

auto timer::transition_to_next_state() -> void {
    const auto old_state{status_.state};
    auto new_state{timer_state::idle};

    switch (status_.state) {
        case timer_state::focus:
            if (status_.current_interval >= status_.total_intervals) {
                new_state = timer_state::long_break;
                status_.current_interval = 1;
            } else {
                new_state = timer_state::short_break;
            }
            break;
        case timer_state::short_break:
            status_.current_interval++;
            new_state = timer_state::focus;
            break;
        case timer_state::long_break:
            new_state = timer_state::focus;
            break;
        default:
            new_state = timer_state::idle;
            active_ = false;
            break;
    }

    status_.state = new_state;
    status_.remaining = calculate_duration(new_state);
    status_.total = status_.remaining;
    status_.one_minute_warning_sent = false;

    if (state_change_callback_) {
        state_change_callback_(old_state, new_state);
    }
}

auto timer::calculate_duration(const timer_state state) const -> std::chrono::seconds {
    switch (state) {
        case timer_state::focus:
            return std::chrono::duration_cast<std::chrono::seconds>(config_.focus_duration);
        case timer_state::short_break:
            return std::chrono::duration_cast<std::chrono::seconds>(config_.short_break_duration);
        case timer_state::long_break:
            return std::chrono::duration_cast<std::chrono::seconds>(config_.long_break_duration);
        default:
            return std::chrono::seconds{0};
    }
}

}
