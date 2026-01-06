#include "daemon.hpp"
#include "service.hpp"
#include "../ipc/protocol.hpp"
#include "../ipc/socket.hpp"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace pominal::daemon {

auto daemon_launcher::start(const timer_config& config) -> bool {
    if (service::is_running()) {
        return false;
    }

    const auto pid{fork()};
    if (pid < 0) {
        return false;
    }

    if (pid > 0) {
        return true;
    }

    if (setsid() < 0) {
        std::exit(1);
    }

    std::signal(SIGCHLD, SIG_IGN);
    std::signal(SIGHUP, SIG_IGN);
    std::signal(SIGPIPE, SIG_IGN);

    const auto child_pid{fork()};
    if (child_pid < 0) {
        std::exit(1);
    }

    if (child_pid > 0) {
        std::exit(0);
    }

    umask(0);
    if (chdir("/") < 0) {
        std::exit(1);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (const auto null_fd{open("/dev/null", O_RDWR)}; null_fd >= 0) {
        dup2(null_fd, STDIN_FILENO);
        dup2(null_fd, STDOUT_FILENO);
        dup2(null_fd, STDERR_FILENO);
        if (null_fd > 2) {
            close(null_fd);
        }
    }

    service svc{config};
    std::exit(svc.run());
}

auto daemon_launcher::stop() -> bool {
    if (!service::is_running()) {
        return false;
    }

    ipc::client_socket client{};
    if (!client.connect_to_server()) {
        return false;
    }

    ipc::request req{};
    req.command = command_type::shutdown;
    if (!client.send_request(req.serialize())) {
        return false;
    }
    (void)client.receive_response();

    return true;
}

auto daemon_launcher::ensure_running(const timer_config& config) -> bool {
    if (service::is_running()) {
        return true;
    }
    if (!start(config)) {
        return false;
    }
    for (auto i{0}; i < 50; ++i) {
        usleep(100000);
        if (service::is_running()) {
            return true;
        }
    }
    return false;
}

}
