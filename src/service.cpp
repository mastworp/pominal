#include "service.hpp"
#include "factory.hpp"
#include "protocol.hpp"
#include "socket.hpp"
#include "sound.hpp"
#include <fstream>
#include <unistd.h>

namespace pominal::daemon {

service::service(const timer_config& config)
    : timer_{config}
    , running_{false} {
    setup_callbacks();
}

auto service::run() -> int {
    ipc::server_socket server{};
    if (!server.bind_and_listen()) {
        return 1;
    }

    write_pid_file();
    running_ = true;

    while (running_) {
        const auto client_fd{server.accept_connection()};
        if (client_fd < 0) {
            continue;
        }

        handle_client(client_fd);
        close(client_fd);
    }

    remove_pid_file();
    return 0;
}

auto service::stop() -> void {
    running_ = false;
}

auto service::is_running() -> bool {
    return ipc::is_server_running();
}

auto service::get_running_pid() -> pid_t {
    std::ifstream file{pid_file_path};
    if (!file) {
        return -1;
    }
    pid_t pid{};
    file >> pid;
    return pid;
}

auto service::setup_callbacks() -> void {
    timer_.set_state_change_callback([this](timer_state /*old_state*/, const timer_state new_state) {
        if (enable_sound_) {
            notification::sound_player::play_for_state(new_state);
        }
    });

    timer_.set_warning_callback([this](const timer_status& /*status*/) {
        if (enable_sound_) {
            notification::sound_player::play_warning();
        }
    });
}

auto service::handle_client(const int client_fd) -> void {
    const auto data{ipc::server_socket::receive_request(client_fd)};
    if (!data) {
        return;
    }

    const auto request{ipc::request::deserialize(*data)};
    if (!request) {
        return;
    }

    if (request->command == command_type::shutdown) {
        ipc::response resp{};
        resp.success = true;
        resp.message = "Service shutting down";
        resp.status = timer_.get_status();
        (void)ipc::server_socket::send_response(client_fd, resp.serialize());
        running_ = false;
        return;
    }

    const auto cmd{command::command_factory::create(request->command)};
    const auto response{cmd->execute(timer_, request->args)};
    (void)ipc::server_socket::send_response(client_fd, response.serialize());
}

auto service::write_pid_file() -> void {
    if (std::ofstream file{pid_file_path}) {
        file << getpid();
    }
}

auto service::remove_pid_file() -> void {
    std::remove(pid_file_path);
}

}
