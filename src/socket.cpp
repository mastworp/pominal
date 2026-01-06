#include "socket.hpp"
#include <filesystem>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace pominal::ipc {

socket_base::~socket_base() {
    close_socket();
}

auto socket_base::is_valid() const -> bool {
    return fd_ >= 0;
}

auto socket_base::close_socket() -> void {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

server_socket::~server_socket() {
    if (is_valid()) {
        std::filesystem::remove(socket_path);
    }
}

auto server_socket::bind_and_listen() -> bool {
    std::filesystem::remove(socket_path);

    fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) {
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close_socket();
        return false;
    }

    if (listen(fd_, 5) < 0) {
        close_socket();
        return false;
    }

    return true;
}

auto server_socket::accept_connection() const -> int {
    return accept(fd_, nullptr, nullptr);
}

auto server_socket::send_response(const int client_fd, const std::string& data) -> bool {
    const auto bytes_sent{send(client_fd, data.c_str(), data.size(), 0)};
    return bytes_sent == static_cast<ssize_t>(data.size());
}

auto server_socket::receive_request(const int client_fd) -> std::optional<std::string> {
    char buffer[buffer_size];
    const auto bytes_received{recv(client_fd, buffer, buffer_size - 1, 0)};
    if (bytes_received <= 0) {
        return std::nullopt;
    }
    buffer[bytes_received] = '\0';
    return std::string{buffer};
}

auto client_socket::connect_to_server() -> bool {
    fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_ < 0) {
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close_socket();
        return false;
    }

    return true;
}

auto client_socket::send_request(const std::string& data) const -> bool {
    const auto bytes_sent{send(fd_, data.c_str(), data.size(), 0)};
    return bytes_sent == static_cast<ssize_t>(data.size());
}

auto client_socket::receive_response() const -> std::optional<std::string> {
    char buffer[buffer_size];
    const auto bytes_received{recv(fd_, buffer, buffer_size - 1, 0)};
    if (bytes_received <= 0) {
        return std::nullopt;
    }
    buffer[bytes_received] = '\0';
    return std::string{buffer};
}

auto is_server_running() -> bool {
    client_socket client{};
    return client.connect_to_server();
}

}
