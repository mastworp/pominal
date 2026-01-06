#pragma once

#include "protocol.hpp"
#include "timer.hpp"
#include <vector>

namespace pominal::command {

class command_interface {
public:
    virtual ~command_interface() = default;
    virtual auto execute(timer& t, const std::vector<std::string>& args) -> ipc::response = 0;
};

class start_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.start();
        resp.message = resp.success ? "Timer started" : "Cannot start timer";
        resp.status = t.get_status();
        return resp;
    }
};

class pause_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.pause();
        resp.message = resp.success ? "Timer paused" : "Cannot pause timer";
        resp.status = t.get_status();
        return resp;
    }
};

class stop_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.stop();
        resp.message = resp.success ? "Timer stopped" : "Cannot stop timer";
        resp.status = t.get_status();
        return resp;
    }
};

class reset_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.reset();
        resp.message = resp.success ? "Timer reset" : "Cannot reset timer";
        resp.status = t.get_status();
        return resp;
    }
};

class skip_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.skip();
        resp.message = resp.success ? "Skipped to next phase" : "Cannot skip";
        resp.status = t.get_status();
        return resp;
    }
};

class status_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = true;
        resp.message = "Status retrieved";
        resp.status = t.get_status();
        return resp;
    }
};

class focus_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.set_focus();
        resp.message = resp.success ? "Focus mode started" : "Cannot start focus mode";
        resp.status = t.get_status();
        return resp;
    }
};

class short_break_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.set_short_break();
        resp.message = resp.success ? "Short break started" : "Cannot start short break";
        resp.status = t.get_status();
        return resp;
    }
};

class long_break_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& /*args*/) -> ipc::response override {
        ipc::response resp{};
        resp.success = t.set_long_break();
        resp.message = resp.success ? "Long break started" : "Cannot start long break";
        resp.status = t.get_status();
        return resp;
    }
};

class interval_command final : public command_interface {
public:
    auto execute(timer& t, const std::vector<std::string>& args) -> ipc::response override {
        ipc::response resp{};
        if (args.empty()) {
            resp.success = false;
            resp.message = "Interval number required";
        } else {
            try {
                const auto interval{static_cast<std::uint8_t>(std::stoi(args[0]))};
                resp.success = t.set_interval(interval);
                resp.message = resp.success ? "Interval set" : "Invalid interval";
            } catch (...) {
                resp.success = false;
                resp.message = "Invalid interval number";
            }
        }
        resp.status = t.get_status();
        return resp;
    }
};

}
