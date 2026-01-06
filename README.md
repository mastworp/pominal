# Pominal

A terminal-based Pomodoro timer service written in modern C++23.

## Features

- Background daemon service with Unix domain socket IPC
- Works across multiple terminal sessions
- Color-coded output (blue for focus, yellow for breaks, magenta for paused)
- Sound notifications for state changes and 1-minute warnings
- Configurable durations for focus, short break, and long break
- Thread-safe, high-performance timer implementation

## Requirements

- macOS or Linux
- CMake 4.1+
- C++23 compatible compiler (Apple Clang 17+ or GCC 13+)
- Ninja (recommended) or Make

## Building

```bash
# Configure (uses Apple Clang via CMakePresets.json)
cmake --preset default

# Build
cmake --build --preset default

# Run tests
./cmake-build-debug/pominal_tests
```

## Installation

Option 1: Copy to PATH
```bash
sudo cp cmake-build-debug/pominal /usr/local/bin/
```

Option 2: Add alias to `~/.zshrc` or `~/.bashrc`
```bash
alias pominal="/path/to/pominal/cmake-build-debug/pominal"
```

## Usage

### Basic Commands

```bash
pominal start          # Start/resume timer
pominal pause          # Pause timer
pominal stop           # Stop and reset
pominal status         # Show current status
pominal skip           # Skip to next phase
pominal reset          # Reset current phase
```

### Direct Mode Switching

```bash
pominal focus          # Start focus session (25 min)
pominal short-break    # Start short break (5 min)
pominal long-break     # Start long break (15 min)
pominal interval 2     # Set interval number (1-4)
```

### Daemon Control

```bash
pominal -d             # Start daemon in background
pominal --foreground   # Run daemon in foreground (debugging)
pominal --stop-daemon  # Stop the background daemon
```

### Custom Durations

```bash
pominal -f 30 -s 10 -l 20 -d   # 30min focus, 10min short, 20min long
```

### Options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message |
| `-d, --daemon` | Start the background service |
| `--stop-daemon` | Stop the background service |
| `--foreground` | Run service in foreground |
| `-f, --focus-time <min>` | Focus duration in minutes (default: 25) |
| `-s, --short-break <min>` | Short break duration in minutes (default: 5) |
| `-l, --long-break <min>` | Long break duration in minutes (default: 15) |
| `-i, --intervals <n>` | Intervals before long break (default: 4) |

### Typical Workflow

1. Open terminal, run `pominal start`
2. Work during focus sessions (blue output)
3. Take breaks when prompted (yellow output)
4. Check status anytime with `pominal status`

The daemon persists across terminal sessions - control it from any terminal window.

---

## Architecture

### Entry Point: `main.cpp`

Everything starts in `main.cpp:7`. The flow is:

1. Parse CLI arguments → `parsed_args` struct
2. Handle special modes (help, stop-daemon, foreground, daemon)
3. Ensure daemon is running
4. Send command via IPC client

### Directory Structure

```
src/
├── cli/          # Command-line interface
├── command/      # Command pattern implementation
├── daemon/       # Background service & daemonization
├── ipc/          # Inter-process communication
├── notification/ # Display & sound output
├── timer/        # Core Pomodoro timer logic
└── utils/        # Shared types and enums
```

### Core Components

#### 1. Timer (`src/timer/timer.hpp`)

The heart of the application:
- State machine with states: `idle`, `focus`, `short_break`, `long_break`, `paused`
- Runs in dedicated thread (`timer_loop()`)
- Uses `steady_clock` for accurate time measurement
- Thread-safe via mutex + condition variable
- Callbacks for state changes, ticks, and 1-minute warnings

#### 2. Command Pattern (`src/command/`)

- `command.hpp`: Abstract `command_interface` with `execute(timer&, args)` method
- `factory.hpp`: `command_factory::create()` produces the right command for each `command_type`
- Decouples command parsing from execution (Open/Closed Principle)

#### 3. IPC Layer (`src/ipc/`)

- `protocol.hpp`: Defines `request` and `response` structures with serialization
- `socket.hpp/cpp`: Unix domain socket at `/tmp/pominal.sock`
- Client sends serialized request → Server deserializes, executes, returns response

#### 4. Daemon (`src/daemon/`)

- `service.cpp`: Runs the main loop accepting client connections
- `daemon.cpp`: Double-fork daemonization (classic Unix pattern)
  - First fork: parent exits, child becomes orphan
  - `setsid()`: Creates new session
  - Second fork: Prevents reacquiring a controlling terminal

### Data Flow

```
┌──────────────────────────────────────────────────────────────┐
│ User Terminal                                                │
│  $ pominal start                                             │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│ CLI Parser (parser.cpp)                                      │
│  - Parses argv into parsed_args struct                       │
│  - Identifies command_type::start                            │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│ Daemon Launcher (daemon.cpp)                                 │
│  - ensure_running() starts daemon if not running             │
│  - Double-fork daemonization for background process          │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│ Client (client.cpp)                                          │
│  - Connects to Unix socket                                   │
│  - Serializes request, sends to daemon                       │
│  - Receives and displays response                            │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│ Service (service.cpp) - Running in daemon process            │
│  - Receives request via socket                               │
│  - Uses command_factory to create command object             │
│  - Executes command on timer instance                        │
│  - Returns response                                          │
└──────────────────────┬───────────────────────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────────────────────┐
│ Timer (timer.cpp)                                            │
│  - timer::start() sets state to focus, starts counting       │
│  - timer_loop thread decrements remaining time each second   │
│  - Fires callbacks on state changes                          │
└──────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Unix domain socket | Fast IPC, works across terminals, simple cleanup |
| Double-fork daemon | Prevents zombie processes, detaches from terminal |
| Command pattern | Open/closed principle - add new commands without modifying existing code |
| State machine timer | Clear transitions, easy to reason about |
| `steady_clock` timing | Immune to system clock changes, accurate intervals |
| `fork()`/`execlp()` for sound | Avoids blocking daemon, cleaner than `std::system()` |

### Thread Safety

The timer uses the **monitor pattern**:
- Single mutex (`mutex_`) protects all state
- Condition variable (`cv_`) for efficient waiting
- Callbacks invoked with lock released to prevent deadlocks

---

## Testing

```bash
# Run all tests
./cmake-build-debug/pominal_tests

# Run specific test suite
./cmake-build-debug/pominal_tests --gtest_filter="timer_test.*"
```

Test suites:
- `timer_test`: Timer state machine and transitions
- `protocol_test`: IPC serialization/deserialization
- `command_test`: Command pattern and factory

## License

MIT
