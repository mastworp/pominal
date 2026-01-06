#pragma once

#include "types.hpp"
#include <string>
#include <unistd.h>

namespace pominal::notification {

class sound_player {
public:
    static auto play_focus_start() -> void {
        play_system_sound("Blow");
    }

    static auto play_break_start() -> void {
        play_system_sound("Glass");
    }

    static auto play_warning() -> void {
        play_system_sound("Purr");
    }

    static auto play_for_state(const timer_state state) -> void {
        switch (state) {
            case timer_state::focus:
                play_focus_start();
                break;
            case timer_state::short_break:
            case timer_state::long_break:
                play_break_start();
                break;
            default:
                break;
        }
    }

private:
    static auto play_system_sound(const std::string& sound_name) -> void {
        if (const auto pid = fork(); pid == 0) {
#ifdef __APPLE__
            const auto path = "/System/Library/Sounds/" + sound_name + ".aiff";
            execlp("afplay", "afplay", path.c_str(), nullptr);
#else
            execlp("paplay", "paplay", "/usr/share/sounds/freedesktop/stereo/complete.oga", nullptr);
#endif
            _exit(0);
        }
    }
};

}
