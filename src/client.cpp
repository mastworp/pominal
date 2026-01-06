#include "client.hpp"
#include "socket.hpp"
#include "display.hpp"

namespace pominal::cli {

auto client::send_command(const command_type cmd, const std::vector<std::string>& args) -> std::optional<ipc::response> {
    ipc::client_socket socket{};
    if (!socket.connect_to_server()) {
        return std::nullopt;
    }

    ipc::request req{};
    req.command = cmd;
    req.args = args;

    if (!socket.send_request(req.serialize())) {
        return std::nullopt;
    }

    const auto data{socket.receive_response()};
    if (!data) {
        return std::nullopt;
    }

    return ipc::response::deserialize(*data);
}

auto client::start() -> bool {
    if (const auto resp{send_command(command_type::start)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::pause() -> bool {
    if (const auto resp{send_command(command_type::pause)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::stop() -> bool {
    if (const auto resp{send_command(command_type::stop)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::reset() -> bool {
    if (const auto resp{send_command(command_type::reset)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::skip() -> bool {
    if (const auto resp{send_command(command_type::skip)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::status() -> bool {
    if (const auto resp{send_command(command_type::status)}) {
        notification::display::print(notification::display::format_status(resp->status));
        return resp->success;
    }
    return false;
}

auto client::focus() -> bool {
    if (const auto resp{send_command(command_type::focus)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::short_break() -> bool {
    if (const auto resp{send_command(command_type::short_break)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::long_break() -> bool {
    if (const auto resp{send_command(command_type::long_break)}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

auto client::interval(const std::uint8_t value) -> bool {
    if (const auto resp{send_command(command_type::interval, {std::to_string(value)})}) {
        notification::display::print(notification::display::format_response(*resp));
        return resp->success;
    }
    return false;
}

}
