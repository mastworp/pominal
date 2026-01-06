#pragma once

#include <optional>
#include <string>

namespace pominal::ipc {

inline constexpr auto socket_path{"/tmp/pominal.sock"};
inline constexpr std::size_t buffer_size{4096};

class socket_base {
public:
    socket_base() = default;
    virtual ~socket_base();

    socket_base(const socket_base&) = delete;
    auto operator=(const socket_base&) -> socket_base& = delete;
    socket_base(socket_base&&) = delete;
    auto operator=(socket_base&&) -> socket_base& = delete;

    [[nodiscard]] auto is_valid() const -> bool;
    auto close_socket() -> void;

protected:
    int fd_{-1};
};

class server_socket final : public socket_base {
public:
    server_socket() = default;
    ~server_socket() override;

    auto bind_and_listen() -> bool;
    [[nodiscard]] auto accept_connection() const -> int;
    [[nodiscard]] static auto send_response(int client_fd, const std::string& data) -> bool;
    [[nodiscard]] static auto receive_request(int client_fd) -> std::optional<std::string>;
};

class client_socket final : public socket_base {
public:
    auto connect_to_server() -> bool;
    [[nodiscard]] auto send_request(const std::string& data) const -> bool;
    [[nodiscard]] auto receive_response() const -> std::optional<std::string>;
};

[[nodiscard]] auto is_server_running() -> bool;

}
