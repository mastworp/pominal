#pragma once

#include "protocol.hpp"
#include "types.hpp"
#include <string>

namespace pominal::notification {

class display {
public:
    struct colors {
        static constexpr auto reset{"\033[0m"};
        static constexpr auto bold{"\033[1m"};
        static constexpr auto dim{"\033[2m"};

        static constexpr auto blue{"\033[34m"};
        static constexpr auto bright_blue{"\033[94m"};
        static constexpr auto yellow{"\033[33m"};
        static constexpr auto bright_yellow{"\033[93m"};
        static constexpr auto green{"\033[32m"};
        static constexpr auto red{"\033[31m"};
        static constexpr auto cyan{"\033[36m"};
        static constexpr auto magenta{"\033[35m"};
        static constexpr auto white{"\033[37m"};

        static constexpr auto bg_blue{"\033[44m"};
        static constexpr auto bg_yellow{"\033[43m"};
        static constexpr auto bg_green{"\033[42m"};
        static constexpr auto bg_red{"\033[41m"};
    };

    [[nodiscard]] static auto format_time(std::chrono::seconds seconds) -> std::string;
    [[nodiscard]] static auto get_state_color(timer_state state) -> const char*;
    [[nodiscard]] static auto get_state_icon(timer_state state) -> std::string;
    [[nodiscard]] static auto format_status(const timer_status& status, bool compact = false) -> std::string;
    [[nodiscard]] static auto format_response(const ipc::response& resp) -> std::string;
    [[nodiscard]] static auto format_warning(const timer_status& status) -> std::string;
    static auto print(const std::string& message) -> void;
};

}
