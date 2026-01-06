#pragma once

#include "types.hpp"

namespace pominal::daemon {

class daemon_launcher {
public:
    [[nodiscard]] static auto start(const timer_config& config = {}) -> bool;
    [[nodiscard]] static auto stop() -> bool;
    [[nodiscard]] static auto ensure_running(const timer_config& config = {}) -> bool;
};

}
