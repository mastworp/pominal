#pragma once

#include "types.hpp"
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace pominal::ipc {

struct request {
    command_type command{command_type::status};
    std::vector<std::string> args{};

    [[nodiscard]] auto serialize() const -> std::string {
        std::ostringstream oss;
        oss << static_cast<int>(command);
        for (const auto& arg : args) {
            oss << "|" << arg;
        }
        oss << "\n";
        return oss.str();
    }

    [[nodiscard]] static auto deserialize(const std::string& data) -> std::optional<request> {
        if (data.empty()) {
            return std::nullopt;
        }

        request req{};
        std::istringstream iss{data};
        std::string token{};

        if (std::getline(iss, token, '|')) {
            try {
                req.command = static_cast<command_type>(std::stoi(token));
            } catch (...) {
                return std::nullopt;
            }
        }

        while (std::getline(iss, token, '|')) {
            if (!token.empty() && token.back() == '\n') {
                token.pop_back();
            }
            req.args.push_back(token);
        }

        return req;
    }
};

struct response {
    bool success{false};
    std::string message{};
    timer_status status{};

    [[nodiscard]] auto serialize() const -> std::string {
        std::ostringstream oss;
        oss << (success ? "1" : "0") << "|"
            << message << "|"
            << static_cast<int>(status.state) << "|"
            << static_cast<int>(status.previous_state) << "|"
            << status.remaining.count() << "|"
            << status.total.count() << "|"
            << static_cast<int>(status.current_interval) << "|"
            << static_cast<int>(status.total_intervals) << "\n";
        return oss.str();
    }

    [[nodiscard]] static auto deserialize(const std::string& data) -> std::optional<response> {
        if (data.empty()) {
            return std::nullopt;
        }

        response resp{};
        std::istringstream iss{data};
        std::string token{};
        std::vector<std::string> tokens{};

        while (std::getline(iss, token, '|')) {
            if (!token.empty() && token.back() == '\n') {
                token.pop_back();
            }
            tokens.push_back(token);
        }

        if (tokens.size() < 8) {
            return std::nullopt;
        }

        try {
            resp.success = tokens[0] == "1";
            resp.message = tokens[1];
            resp.status.state = static_cast<timer_state>(std::stoi(tokens[2]));
            resp.status.previous_state = static_cast<timer_state>(std::stoi(tokens[3]));
            resp.status.remaining = std::chrono::seconds{std::stoll(tokens[4])};
            resp.status.total = std::chrono::seconds{std::stoll(tokens[5])};
            resp.status.current_interval = static_cast<std::uint8_t>(std::stoi(tokens[6]));
            resp.status.total_intervals = static_cast<std::uint8_t>(std::stoi(tokens[7]));
        } catch (...) {
            return std::nullopt;
        }

        return resp;
    }
};

}
