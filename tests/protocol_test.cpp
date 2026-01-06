#include <gtest/gtest.h>
#include "protocol.hpp"

using namespace pominal;
using namespace pominal::ipc;

class protocol_test : public testing::Test {};

TEST_F(protocol_test, request_serialization_roundtrip) {
    request req{};
    req.command = command_type::start;
    req.args = {"arg1", "arg2"};

    const auto serialized{req.serialize()};
    const auto deserialized{request::deserialize(serialized)};

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->command, command_type::start);
    ASSERT_EQ(deserialized->args.size(), 2);
    EXPECT_EQ(deserialized->args[0], "arg1");
    EXPECT_EQ(deserialized->args[1], "arg2");
}

TEST_F(protocol_test, request_serialization_no_args) {
    request req{};
    req.command = command_type::status;

    const auto serialized{req.serialize()};
    const auto deserialized{request::deserialize(serialized)};

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->command, command_type::status);
    EXPECT_TRUE(deserialized->args.empty());
}

TEST_F(protocol_test, request_deserialize_empty_returns_nullopt) {
    const auto result{request::deserialize("")};
    EXPECT_FALSE(result.has_value());
}

TEST_F(protocol_test, response_serialization_roundtrip) {
    response resp{};
    resp.success = true;
    resp.message = "Timer started";
    resp.status.state = timer_state::focus;
    resp.status.previous_state = timer_state::idle;
    resp.status.remaining = std::chrono::seconds{1500};
    resp.status.total = std::chrono::seconds{1500};
    resp.status.current_interval = 2;
    resp.status.total_intervals = 4;

    auto serialized{resp.serialize()};
    auto deserialized{response::deserialize(serialized)};

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_TRUE(deserialized->success);
    EXPECT_EQ(deserialized->message, "Timer started");
    EXPECT_EQ(deserialized->status.state, timer_state::focus);
    EXPECT_EQ(deserialized->status.previous_state, timer_state::idle);
    EXPECT_EQ(deserialized->status.remaining.count(), 1500);
    EXPECT_EQ(deserialized->status.total.count(), 1500);
    EXPECT_EQ(deserialized->status.current_interval, 2);
    EXPECT_EQ(deserialized->status.total_intervals, 4);
}

TEST_F(protocol_test, response_serialization_failure_case) {
    response resp{};
    resp.success = false;
    resp.message = "Cannot start timer";

    auto serialized{resp.serialize()};
    auto deserialized{response::deserialize(serialized)};

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_FALSE(deserialized->success);
    EXPECT_EQ(deserialized->message, "Cannot start timer");
}

TEST_F(protocol_test, response_deserialize_empty_returns_nullopt) {
    const auto result{response::deserialize("")};
    EXPECT_FALSE(result.has_value());
}

TEST_F(protocol_test, response_deserialize_insufficient_fields_returns_nullopt) {
    const auto result{response::deserialize("1|message|0")};
    EXPECT_FALSE(result.has_value());
}

TEST_F(protocol_test, all_command_types_serialize_correctly) {
    const std::vector commands{
        command_type::start,
        command_type::pause,
        command_type::stop,
        command_type::reset,
        command_type::skip,
        command_type::status,
        command_type::focus,
        command_type::short_break,
        command_type::long_break,
        command_type::interval,
        command_type::shutdown
    };

    for (auto cmd : commands) {
        request req{};
        req.command = cmd;
        auto serialized{req.serialize()};
        auto deserialized{request::deserialize(serialized)};
        ASSERT_TRUE(deserialized.has_value());
        EXPECT_EQ(deserialized->command, cmd);
    }
}

TEST_F(protocol_test, all_timer_states_serialize_correctly) {
    std::vector states{
        timer_state::idle,
        timer_state::focus,
        timer_state::short_break,
        timer_state::long_break,
        timer_state::paused
    };

    for (auto state : states) {
        response resp{};
        resp.success = true;
        resp.message = "test";
        resp.status.state = state;
        resp.status.previous_state = timer_state::idle;
        resp.status.remaining = std::chrono::seconds{100};
        resp.status.total = std::chrono::seconds{100};
        resp.status.current_interval = 1;
        resp.status.total_intervals = 4;

        auto serialized{resp.serialize()};
        auto deserialized{response::deserialize(serialized)};
        ASSERT_TRUE(deserialized.has_value());
        EXPECT_EQ(deserialized->status.state, state);
    }
}
