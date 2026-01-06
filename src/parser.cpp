#include "parser.hpp"
#include <chrono>
#include <iostream>

namespace pominal::cli {

auto argument_parser::parse(const int argc, char* argv[]) -> parsed_args {
    parsed_args result{};
    std::vector<std::string> args{};

    for (auto i{1}; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    for (std::size_t i{0}; i < args.size(); ++i) {
        const auto& arg{args[i]};

        if (arg == "-h" || arg == "--help") {
            result.show_help = true;
            continue;
        }

        if (arg == "-d" || arg == "--daemon") {
            result.daemon_mode = true;
            continue;
        }

        if (arg == "--stop-daemon") {
            result.stop_daemon = true;
            continue;
        }

        if (arg == "--foreground") {
            result.foreground_mode = true;
            continue;
        }

        if (arg == "-f" || arg == "--focus-time") {
            if (i + 1 < args.size()) {
                result.config.focus_duration = std::chrono::minutes{std::stoi(args[++i])};
            }
            continue;
        }

        if (arg == "-s" || arg == "--short-break") {
            if (i + 1 < args.size()) {
                result.config.short_break_duration = std::chrono::minutes{std::stoi(args[++i])};
            }
            continue;
        }

        if (arg == "-l" || arg == "--long-break") {
            if (i + 1 < args.size()) {
                result.config.long_break_duration = std::chrono::minutes{std::stoi(args[++i])};
            }
            continue;
        }

        if (arg == "-i" || arg == "--intervals") {
            if (i + 1 < args.size()) {
                result.config.intervals_before_long_break = static_cast<std::uint8_t>(std::stoi(args[++i]));
            }
            continue;
        }

        if (!result.command) {
            result.command = string_to_command(arg);
        } else {
            result.extra_args.push_back(arg);
        }
    }

    return result;
}

auto argument_parser::print_help() -> void {
    std::cout << R"(
pominal - A terminal-based Pomodoro timer service

USAGE:
    pominal [OPTIONS] [COMMAND]

COMMANDS:
    start           Start the timer (or resume if paused)
    pause           Pause the current timer
    stop            Stop and reset the timer
    reset           Reset current phase to beginning
    skip            Skip to next phase
    status          Show current timer status
    focus           Start a focus session
    short-break     Start a short break
    long-break      Start a long break
    interval <n>    Set current interval number

OPTIONS:
    -h, --help              Show this help message
    -d, --daemon            Start the background service
    --stop-daemon           Stop the background service
    --foreground            Run service in foreground (for debugging)
    -f, --focus-time <min>  Focus duration in minutes (default: 25)
    -s, --short-break <min> Short break duration in minutes (default: 5)
    -l, --long-break <min>  Long break duration in minutes (default: 15)
    -i, --intervals <n>     Intervals before long break (default: 4)

EXAMPLES:
    pominal -d                      Start the daemon
    pominal start                   Start the timer
    pominal status                  Check timer status
    pominal -f 30 -s 10 -d          Start daemon with custom times

)";
}

}
