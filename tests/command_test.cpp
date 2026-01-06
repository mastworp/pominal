#include <gtest/gtest.h>
#include "src/command/command.hpp"
#include "src/command/factory.hpp"
#include "src/timer/timer.hpp"

using namespace pominal;
using namespace pominal::command;

class command_test : public testing::Test {
protected:
    static auto create_fast_config() -> timer_config {
        timer_config config{};
        config.focus_duration = std::chrono::minutes{1};
        config.short_break_duration = std::chrono::minutes{1};
        config.long_break_duration = std::chrono::minutes{1};
        config.intervals_before_long_break = 4;
        return config;
    }
};

TEST_F(command_test, factory_creates_start_command) {
    const auto cmd{command_factory::create(command_type::start)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_pause_command) {
    const auto cmd{command_factory::create(command_type::pause)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_stop_command) {
    const auto cmd{command_factory::create(command_type::stop)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_reset_command) {
    const auto cmd{command_factory::create(command_type::reset)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_skip_command) {
    const auto cmd{command_factory::create(command_type::skip)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_status_command) {
    const auto cmd{command_factory::create(command_type::status)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_focus_command) {
    const auto cmd{command_factory::create(command_type::focus)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_short_break_command) {
    const auto cmd{command_factory::create(command_type::short_break)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_long_break_command) {
    const auto cmd{command_factory::create(command_type::long_break)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, factory_creates_interval_command) {
    const auto cmd{command_factory::create(command_type::interval)};
    EXPECT_NE(cmd, nullptr);
}

TEST_F(command_test, start_command_executes) {
    timer t{create_fast_config()};
    start_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::focus);
}

TEST_F(command_test, pause_command_fails_when_idle) {
    timer t{create_fast_config()};
    pause_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_FALSE(response.success);
}

TEST_F(command_test, pause_command_succeeds_when_running) {
    timer t{create_fast_config()};
    t.start();
    pause_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::paused);
}

TEST_F(command_test, stop_command_executes) {
    timer t{create_fast_config()};
    t.start();
    stop_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::idle);
}

TEST_F(command_test, reset_command_fails_when_idle) {
    timer t{create_fast_config()};
    reset_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_FALSE(response.success);
}

TEST_F(command_test, reset_command_succeeds_when_running) {
    timer t{create_fast_config()};
    t.start();
    reset_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
}

TEST_F(command_test, skip_command_fails_when_idle) {
    timer t{create_fast_config()};
    skip_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_FALSE(response.success);
}

TEST_F(command_test, skip_command_transitions_focus_to_short_break) {
    timer t{create_fast_config()};
    t.start();
    skip_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::short_break);
}

TEST_F(command_test, status_command_always_succeeds) {
    timer t{create_fast_config()};
    status_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
}

TEST_F(command_test, focus_command_starts_focus) {
    timer t{create_fast_config()};
    focus_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::focus);
}

TEST_F(command_test, short_break_command_starts_short_break) {
    timer t{create_fast_config()};
    short_break_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::short_break);
}

TEST_F(command_test, long_break_command_starts_long_break) {
    timer t{create_fast_config()};
    long_break_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.state, timer_state::long_break);
}

TEST_F(command_test, interval_command_without_args_fails) {
    timer t{create_fast_config()};
    interval_command cmd{};
    const auto response{cmd.execute(t, {})};
    EXPECT_FALSE(response.success);
}

TEST_F(command_test, interval_command_with_valid_arg_succeeds) {
    timer t{create_fast_config()};
    interval_command cmd{};
    const auto response{cmd.execute(t, {"2"})};
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.status.current_interval, 2);
}

TEST_F(command_test, interval_command_with_invalid_arg_fails) {
    timer t{create_fast_config()};
    interval_command cmd{};
    const auto response{cmd.execute(t, {"invalid"})};
    EXPECT_FALSE(response.success);
}

TEST_F(command_test, interval_command_with_out_of_range_fails) {
    timer t{create_fast_config()};
    interval_command cmd{};
    const auto response{cmd.execute(t, {"10"})};
    EXPECT_FALSE(response.success);
}
