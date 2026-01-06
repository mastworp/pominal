#pragma once

#include "../utils/types.hpp"
#include <optional>
#include <string>
#include <vector>

namespace pominal::cli {

struct parsed_args {
    std::optional<command_type> command{};
    timer_config config{};
    std::vector<std::string> extra_args{};
    bool show_help{false};
    bool daemon_mode{false};
    bool stop_daemon{false};
    bool foreground_mode{false};
};

class argument_parser {
public:
    [[nodiscard]] static auto parse(int argc, char* argv[]) -> parsed_args;
    static auto print_help() -> void;
};

}
