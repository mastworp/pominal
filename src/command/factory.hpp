#pragma once

#include "command.hpp"
#include <memory>

namespace pominal::command {

class command_factory {
public:
    static auto create(const command_type type) -> std::unique_ptr<command_interface> {
        switch (type) {
            case command_type::start:
                return std::make_unique<start_command>();
            case command_type::pause:
                return std::make_unique<pause_command>();
            case command_type::stop:
                return std::make_unique<stop_command>();
            case command_type::reset:
                return std::make_unique<reset_command>();
            case command_type::skip:
                return std::make_unique<skip_command>();
            case command_type::status:
                return std::make_unique<status_command>();
            case command_type::focus:
                return std::make_unique<focus_command>();
            case command_type::short_break:
                return std::make_unique<short_break_command>();
            case command_type::long_break:
                return std::make_unique<long_break_command>();
            case command_type::interval:
                return std::make_unique<interval_command>();
            default:
                return std::make_unique<status_command>();
        }
    }
};

}
