#pragma once

#include "protocol.hpp"
#include <optional>
#include <string>
#include <vector>

namespace pominal::cli {

class client {
public:
    [[nodiscard]] static auto send_command(command_type cmd, const std::vector<std::string>& args = {}) -> std::optional<ipc::response>;
    [[nodiscard]] static auto start() -> bool;
    [[nodiscard]] static auto pause() -> bool;
    [[nodiscard]] static auto stop() -> bool;
    [[nodiscard]] static auto reset() -> bool;
    [[nodiscard]] static auto skip() -> bool;
    [[nodiscard]] static auto status() -> bool;
    [[nodiscard]] static auto focus() -> bool;
    [[nodiscard]] static auto short_break() -> bool;
    [[nodiscard]] static auto long_break() -> bool;
    [[nodiscard]] static auto interval(std::uint8_t value) -> bool;
};

}
