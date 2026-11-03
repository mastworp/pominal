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
- CMake 3.25+
- C++23 compatible compiler (Apple Clang 17+ or GCC 13+)
- Ninja
- Optional: clang-format 22, cmake-format 0.6.13 and gcovr 8 for formatting and coverage (all included in the Docker toolchain)

## Building

Every build is described by a preset in `CMakePresets.json`. Build trees are created in `build/<preset>`.

```bash
cmake --workflow --preset debug      # configure, build and test a Debug build
cmake --workflow --preset release    # configure, build and test a Release build
```

The individual steps are available as well:

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

| Workflow preset | What it does |
|-----------------|--------------|
| `debug` | Debug build and tests |
| `release` | Release build and tests |
| `install` | Release build and tests, then installs into `~/.local` |
| `coverage` | Instrumented build, tests, coverage report and minimum coverage check |
| `package` | Static release build, tests and `.tar.gz`, `.deb` and `.rpm` packages (Linux only) |
| `format` | Formats all C++ and CMake sources |
| `format-check` | Fails when a source file is not formatted |

## Installation

```bash
cmake --workflow --preset install                         # installs ~/.local/bin/pominal
sudo cmake --install build/release --prefix /usr/local    # system-wide, after a release build
```

Or add an alias to `~/.zshrc` or `~/.bashrc`:
```bash
alias pominal="/path/to/pominal/build/release/pominal"
```

### Linux packages

Every `v*` tag publishes statically linked packages for x86_64 and aarch64 on the GitHub releases page. They have no runtime dependencies and install on any distribution:

```bash
sudo apt install ./pominal_1.0.0_amd64.deb                                         # Debian, Ubuntu
sudo dnf install ./pominal-1.0.0-1.x86_64.rpm                                      # Fedora, RHEL, AlmaLinux
sudo zypper install --allow-unsigned-rpm ./pominal-1.0.0-1.x86_64.rpm              # openSUSE
sudo tar -xzf pominal-1.0.0-linux-x86_64.tar.gz -C /usr/local --strip-components=1 # any distribution
```

On aarch64 the files are named `pominal_1.0.0_arm64.deb`, `pominal-1.0.0-1.aarch64.rpm` and `pominal-1.0.0-linux-aarch64.tar.gz`. The packages are not signed, so on RPM 6 based systems such as Fedora 45 a plain `rpm -i` needs `--nosignature`.

To build the packages yourself on any host with Docker:

```bash
docker buildx build --target artifacts --output type=local,dest=dist .
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

Everything starts in `main.cpp:8`. The flow is:

1. Parse CLI arguments → `parsed_args` struct
2. Handle special modes (help, stop-daemon, foreground, daemon)
3. Ensure daemon is running
4. Send command via IPC client

### Directory Structure

```
pominal/
├── include/           # All headers (flat)
│   ├── types.hpp      # Shared types and enums
│   ├── timer.hpp      # Core Pomodoro timer
│   ├── protocol.hpp   # IPC request/response structures
│   ├── socket.hpp     # Unix domain socket wrapper
│   ├── command.hpp    # Command pattern interface
│   ├── factory.hpp    # Command factory
│   ├── sound.hpp      # Sound notifications
│   ├── display.hpp    # Terminal display formatting
│   ├── service.hpp    # Background service
│   ├── daemon.hpp     # Daemonization logic
│   ├── client.hpp     # CLI client
│   └── parser.hpp     # Argument parser
├── src/               # All sources (flat)
│   ├── timer.cpp
│   ├── socket.cpp
│   ├── display.cpp
│   ├── service.cpp
│   ├── daemon.cpp
│   ├── client.cpp
│   └── parser.cpp
├── tests/             # Unit tests
│   ├── timer_test.cpp
│   ├── protocol_test.cpp
│   └── command_test.cpp
├── cmake/             # CMake modules
│   ├── Coverage.cmake # gcov instrumentation and gcovr report
│   ├── Format.cmake   # format and format-check targets
│   └── Packaging.cmake # CPack TGZ/DEB/RPM configuration
├── .github/           # CI/CD pipeline and toolchain action
├── main.cpp           # Entry point
├── CMakeLists.txt
├── CMakePresets.json
├── Dockerfile         # Fedora toolchain, build, artifacts and runtime stages
├── docker-compose.yml
├── .clang-format
├── .cmake-format.yaml
└── README.md
```

### Core Components

#### 1. Timer (`include/timer.hpp`)

The heart of the application:
- State machine with states: `idle`, `focus`, `short_break`, `long_break`, `paused`
- Runs in dedicated thread (`timer_loop()`)
- Uses `steady_clock` for accurate time measurement
- Thread-safe via mutex + condition variable
- Callbacks for state changes, ticks, and 1-minute warnings

#### 2. Command Pattern (`include/command.hpp`, `include/factory.hpp`)

- `command.hpp`: Abstract `command_interface` with `execute(timer&, args)` method
- `factory.hpp`: `command_factory::create()` produces the right command for each `command_type`
- Decouples command parsing from execution (Open/Closed Principle)

#### 3. IPC Layer (`include/protocol.hpp`, `include/socket.hpp`)

- `protocol.hpp`: Defines `request` and `response` structures with serialization
- `socket.hpp`: Unix domain socket wrapper at `/tmp/pominal.sock`
- Client sends serialized request → Server deserializes, executes, returns response

#### 4. Daemon (`include/daemon.hpp`, `include/service.hpp`)

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
ctest --preset debug

# Run specific test suite
ctest --preset debug -R timer_test
./build/debug/pominal_tests --gtest_filter="timer_test.*"
```

Test suites:
- `timer_test`: Timer state machine and transitions
- `protocol_test`: IPC serialization/deserialization
- `command_test`: Command pattern and factory
- `cli.help`: Smoke test that runs the `pominal` executable

## Code Coverage

```bash
cmake --workflow --preset coverage
```

The `coverage` preset instruments the library and tests with gcov and generates an HTML, Cobertura XML and Markdown report with gcovr in `build/coverage/coverage-report`. The build fails when line coverage drops below `POMINAL_COVERAGE_MIN_LINES` (default `40`), which CI enforces on every push and pull request. Raise it as tests are added:

```bash
cmake --preset coverage -DPOMINAL_COVERAGE_MIN_LINES=50
```

## Code Formatting

C++ sources follow `.clang-format`, CMake files follow `.cmake-format.yaml`. CI checks formatting with clang-format 22.1.2 and cmake-format 0.6.13 (pinned in the `Dockerfile`).

```bash
cmake --workflow --preset format          # rewrite all sources
cmake --workflow --preset format-check    # verify only
```

cmake-format does not run on Python 3.15 or newer, which is why the toolchain installs it into a Python 3.14 environment.

## Docker

The `Dockerfile` is based on Fedora 45 and has four targets:

| Target | Contents |
|--------|----------|
| `toolchain` | GCC, CMake, Ninja, clang-format, cmake-format, gcovr and rpmbuild |
| `build` | Builds, tests and packages the sources |
| `artifacts` | Only the packages, for export with `--output` |
| `runtime` | Fedora with the pominal RPM installed, running the service in the foreground |

```bash
docker compose run --rm dev                                        # shell in the toolchain, sources mounted at /workspace
docker compose run --rm dev cmake --workflow --preset coverage     # run any preset on Fedora
docker compose up -d pominal                                       # run the service in a container
docker compose exec pominal pominal start
docker compose exec pominal pominal status
docker compose down
```

The `dev` service keeps its build trees in a named volume so they do not clash with host builds in `build/`.

## CI/CD

`.github/workflows/ci.yml` runs on pushes to `master`, pull requests and `v*` tags:

| Job | Purpose |
|-----|---------|
| Format | `format-check` workflow in the Fedora toolchain |
| Build & test (Fedora, GCC) | `release` workflow in the Fedora toolchain |
| Build & test (macOS, Apple Clang) | `release` workflow on `macos-latest` |
| Coverage | `coverage` workflow including the minimum coverage check, uploads the HTML report and adds a summary to the run |
| Package | Builds the static packages from the `Dockerfile` on x86_64 and arm64 runners, checks the runtime image and uploads the packages |
| Install on ... | Installs the packages on Debian, Ubuntu, Fedora, AlmaLinux, openSUSE and Alpine on both architectures, plus Arch Linux on x86_64, and runs a daemon round trip |
| Release | On `v*` tags, checks that the tag matches the project version and publishes the packages of both architectures with `SHA256SUMS` as a GitHub release |

The coverage report and package artifacts of each run are kept for 7 days. Released packages stay on the releases page.

To publish a release, bump `VERSION` in `CMakeLists.txt`, then push a matching tag:

```bash
git tag v1.0.1
git push origin v1.0.1
```

## License

MIT
