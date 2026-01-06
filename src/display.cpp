#include "display.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>

namespace pominal::notification {

auto display::format_time(const std::chrono::seconds seconds) -> std::string {
    const auto mins{std::chrono::duration_cast<std::chrono::minutes>(seconds).count()};
    const auto secs{(seconds - std::chrono::minutes{mins}).count()};
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << mins << ":"
        << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

auto display::get_state_color(const timer_state state) -> const char* {
    switch (state) {
        case timer_state::focus:
            return colors::bright_blue;
        case timer_state::short_break:
        case timer_state::long_break:
            return colors::bright_yellow;
        case timer_state::paused:
            return colors::magenta;
        case timer_state::idle:
        default:
            return colors::dim;
    }
}

auto display::get_state_icon(const timer_state state) -> std::string {
    switch (state) {
        case timer_state::focus:
            return "🎯";
        case timer_state::short_break:
            return "☕";
        case timer_state::long_break:
            return "🌴";
        case timer_state::paused:
            return "⏸";
        case timer_state::idle:
        default:
            return "💤";
    }
}

auto display::format_status(const timer_status& status, const bool compact) -> std::string {
    std::ostringstream oss;
    const auto color{get_state_color(status.state)};
    const auto icon{get_state_icon(status.state)};

    if (compact) {
        oss << color << icon << " " << format_time(status.remaining) << colors::reset;
    } else {
        oss << "\n"
            << color << colors::bold
            << "╔══════════════════════════════╗\n"
            << "║  " << icon << " POMINAL                  ║\n"
            << "╠══════════════════════════════╣\n"
            << "║  State: " << std::setw(20) << std::left << state_to_string(status.state) << " ║\n"
            << "║  Time:  " << std::setw(20) << std::left << format_time(status.remaining) << " ║\n"
            << "║  Interval: " << static_cast<int>(status.current_interval)
            << "/" << static_cast<int>(status.total_intervals) << "               ║\n"
            << "╚══════════════════════════════╝"
            << colors::reset << "\n";
    }
    return oss.str();
}

auto display::format_response(const ipc::response& resp) -> std::string {
    std::ostringstream oss;
    const auto color{resp.success ? colors::green : colors::red};
    const auto status_color{get_state_color(resp.status.state)};

    oss << color << (resp.success ? "✓ " : "✗ ") << resp.message << colors::reset << "\n";

    if (resp.status.state != timer_state::idle) {
        oss << status_color
            << get_state_icon(resp.status.state) << " "
            << state_to_string(resp.status.state) << " - "
            << format_time(resp.status.remaining)
            << " [" << static_cast<int>(resp.status.current_interval)
            << "/" << static_cast<int>(resp.status.total_intervals) << "]"
            << colors::reset << "\n";
    }

    return oss.str();
}

auto display::format_warning(const timer_status& status) -> std::string {
    std::ostringstream oss;
    const auto color{get_state_color(status.state)};
    oss << "\n" << colors::bold << color
        << "⚠️  1 minute remaining in " << state_to_string(status.state)
        << colors::reset << "\n";
    return oss.str();
}

auto display::print(const std::string& message) -> void {
    std::cout << message << std::flush;
}

}
