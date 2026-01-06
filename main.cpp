#include "client.hpp"
#include "parser.hpp"
#include "daemon.hpp"
#include "service.hpp"
#include "display.hpp"
#include <iostream>

auto main(const int argc, char* argv[]) -> int {
    using namespace pominal;

    auto [command, config, extra_args, show_help, daemon_mode, stop_daemon, foreground_mode] = cli::argument_parser::parse(argc, argv);

    if (show_help) {
        cli::argument_parser::print_help();
        return 0;
    }

    if (stop_daemon) {
        if (daemon::daemon_launcher::stop()) {
            std::cout << notification::display::colors::green
                      << "Daemon stopped successfully"
                      << notification::display::colors::reset << "\n";
            return 0;
        }
        std::cerr << notification::display::colors::red
                  << "Failed to stop daemon (not running?)"
                  << notification::display::colors::reset << "\n";
        return 1;
    }

    if (foreground_mode) {
        if (daemon::service::is_running()) {
            std::cerr << notification::display::colors::red
                      << "Another instance is already running"
                      << notification::display::colors::reset << "\n";
            return 1;
        }
        std::cout << notification::display::colors::green
                  << "Starting service in foreground..."
                  << notification::display::colors::reset << "\n";
        daemon::service svc{config};
        return svc.run();
    }

    if (daemon_mode) {
        if (daemon::service::is_running()) {
            std::cout << notification::display::colors::yellow
                      << "Daemon is already running"
                      << notification::display::colors::reset << "\n";
            return 0;
        }
        if (daemon::daemon_launcher::start(config)) {
            std::cout << notification::display::colors::green
                      << "Daemon started successfully"
                      << notification::display::colors::reset << "\n";
            return 0;
        }
        std::cerr << notification::display::colors::red
                  << "Failed to start daemon"
                  << notification::display::colors::reset << "\n";
        return 1;
    }

    if (!daemon::daemon_launcher::ensure_running(config)) {
        std::cerr << notification::display::colors::red
                  << "Failed to start daemon service"
                  << notification::display::colors::reset << "\n";
        return 1;
    }

    if (!command) {
        (void)cli::client::status();
        return 0;
    }

    switch (*command) {
        case command_type::start:
            (void)cli::client::start();
            break;
        case command_type::pause:
            (void)cli::client::pause();
            break;
        case command_type::stop:
            (void)cli::client::stop();
            break;
        case command_type::reset:
            (void)cli::client::reset();
            break;
        case command_type::skip:
            (void)cli::client::skip();
            break;
        case command_type::status:
            (void)cli::client::status();
            break;
        case command_type::focus:
            (void)cli::client::focus();
            break;
        case command_type::short_break:
            (void)cli::client::short_break();
            break;
        case command_type::long_break:
            (void)cli::client::long_break();
            break;
        case command_type::interval:
            if (!extra_args.empty()) {
                (void)cli::client::interval(static_cast<std::uint8_t>(std::stoi(extra_args[0])));
            } else {
                std::cerr << "Interval command requires a number argument\n";
                return 1;
            }
            break;
        default:
            (void)cli::client::status();
            break;
    }

    return 0;
}
