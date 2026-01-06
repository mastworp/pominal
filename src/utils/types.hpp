#pragma once

#include <chrono>
#include <string>

namespace pominal {

enum class timer_state : std::uint8_t {
    idle,
    focus,
    short_break,
    long_break,
    paused
};

enum class command_type : std::uint8_t {
    start,
    pause,
    stop,
    reset,
    skip,
    status,
    focus,
    short_break,
    long_break,
    interval,
    shutdown
};

struct timer_config {
    std::chrono::minutes focus_duration{25};
    std::chrono::minutes short_break_duration{5};
    std::chrono::minutes long_break_duration{15};
    std::uint8_t intervals_before_long_break{4};
};

struct timer_status {
    timer_state state{timer_state::idle};
    timer_state previous_state{timer_state::idle};
    std::chrono::seconds remaining{0};
    std::chrono::seconds total{0};
    std::uint8_t current_interval{1};
    std::uint8_t total_intervals{4};
    bool one_minute_warning_sent{false};
};

inline auto state_to_string(const timer_state state) -> std::string {
    switch (state) {
        case timer_state::idle: return "idle";
        case timer_state::focus: return "focus";
        case timer_state::short_break: return "short_break";
        case timer_state::long_break: return "long_break";
        case timer_state::paused: return "paused";
    }
    return "unknown";
}

inline auto string_to_command(const std::string& cmd) -> command_type {
    if (cmd == "start") return command_type::start;
    if (cmd == "pause") return command_type::pause;
    if (cmd == "stop") return command_type::stop;
    if (cmd == "reset") return command_type::reset;
    if (cmd == "skip") return command_type::skip;
    if (cmd == "status") return command_type::status;
    if (cmd == "focus") return command_type::focus;
    if (cmd == "short-break") return command_type::short_break;
    if (cmd == "long-break") return command_type::long_break;
    if (cmd == "interval") return command_type::interval;
    if (cmd == "shutdown") return command_type::shutdown;
    return command_type::status;
}

}
