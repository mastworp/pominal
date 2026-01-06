#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "src/timer/timer.hpp"
#include <chrono>
#include <thread>

using namespace pominal;
using namespace std::chrono_literals;

class timer_test : public testing::Test {
protected:
    static auto create_fast_config() -> timer_config {
        timer_config config{};
        config.focus_duration = 1min;
        config.short_break_duration = 1min;
        config.long_break_duration = 1min;
        config.intervals_before_long_break = 4;
        return config;
    }
};

TEST_F(timer_test, initial_state_is_idle) {
    const timer t{create_fast_config()};
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::idle);
}

TEST_F(timer_test, start_transitions_from_idle_to_focus) {
    timer t{create_fast_config()};
    const auto result{t.start()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::focus);
}

TEST_F(timer_test, pause_transitions_to_paused) {
    timer t{create_fast_config()};
    t.start();
    const auto result{t.pause()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::paused);
}

TEST_F(timer_test, pause_from_idle_fails) {
    timer t{create_fast_config()};
    const auto result{t.pause()};
    EXPECT_FALSE(result);
}

TEST_F(timer_test, stop_resets_timer) {
    timer t{create_fast_config()};
    t.start();
    const auto result{t.stop()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::idle);
    EXPECT_EQ(status.remaining.count(), 0);
}

TEST_F(timer_test, reset_restores_current_phase_time) {
    timer t{create_fast_config()};
    t.start();
    std::this_thread::sleep_for(100ms);
    const auto before_reset{t.get_status()};
    t.reset();
    const auto after_reset{t.get_status()};
    EXPECT_GE(after_reset.remaining, before_reset.remaining);
}

TEST_F(timer_test, set_focus_starts_focus_mode) {
    timer t{create_fast_config()};
    const auto result{t.set_focus()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::focus);
}

TEST_F(timer_test, set_short_break_starts_short_break) {
    timer t{create_fast_config()};
    const auto result{t.set_short_break()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::short_break);
}

TEST_F(timer_test, set_long_break_starts_long_break) {
    timer t{create_fast_config()};
    const auto result{t.set_long_break()};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::long_break);
}

TEST_F(timer_test, set_interval_within_range_succeeds) {
    timer t{create_fast_config()};
    const auto result{t.set_interval(3)};
    EXPECT_TRUE(result);
    const auto status{t.get_status()};
    EXPECT_EQ(status.current_interval, 3);
}

TEST_F(timer_test, set_interval_zero_fails) {
    timer t{create_fast_config()};
    const auto result{t.set_interval(0)};
    EXPECT_FALSE(result);
}

TEST_F(timer_test, set_interval_above_max_fails) {
    timer t{create_fast_config()};
    const auto result{t.set_interval(10)};
    EXPECT_FALSE(result);
}

TEST_F(timer_test, resume_from_pause_restores_previous_state) {
    timer t{create_fast_config()};
    t.set_focus();
    t.pause();
    t.start();
    const auto status{t.get_status()};
    EXPECT_EQ(status.state, timer_state::focus);
}

TEST_F(timer_test, config_can_be_updated) {
    timer t{};
    timer_config new_config{};
    new_config.focus_duration = 30min;
    t.update_config(new_config);
    const auto config{t.get_config()};
    EXPECT_EQ(config.focus_duration, 30min);
}

TEST_F(timer_test, state_change_callback_is_invoked) {
    timer t{create_fast_config()};
    bool callback_called{false};
    auto old_captured{timer_state::idle};
    auto new_captured{timer_state::idle};

    t.set_state_change_callback([&](const timer_state old_state, const timer_state new_state) {
        callback_called = true;
        old_captured = old_state;
        new_captured = new_state;
    });

    t.start();

    EXPECT_TRUE(callback_called);
    EXPECT_EQ(old_captured, timer_state::idle);
    EXPECT_EQ(new_captured, timer_state::focus);
}
