### Quick orientation — what this repo is

This repo contains an Arduino/PlatformIO-based robot firmware (FollowBot) plus small native tests and a simple frontend interface. The firmware runs on the `uno_r4_wifi` (renesas-ra/Arduino framework) and the codebase is organized by capability (UI, sensors, motors, GPS, ROS2 serial bridge, etc.).

### Big-picture architecture

- Entry point: `FollowBot/src/main.cpp` — calls `followBotManager.followBotSetup()` and `followBotManager.followBotLoop()`.
- Central coordinator: `FollowBot/src/followbot_manager/FollowBotManager.*` — constructs and orchestrates subsystems (LCD, GPS, motors, sensors, client, ROS2 bridge).
- Subsystems live under `src/` grouped by responsibility: `followbot_client/`, `followbot_ui/`, `sensors/`, `motors/`, `gps/`, `ROS2_Serial/`, `following_mechanics/`, `motion/`, `secrets/`.
- Cross-cutting pattern: many modules define a single global instance in their `.cpp` (e.g., `FollowBotManager followBotManager;`, `ROS2_Serial ros2_serial;`, `Motion::getInstance()` singleton). Expect global singletons instead of DI in many places.

Why this matters for an AI code agent: changes that affect global objects must account for their module-level construction order. Refactors that convert globals to injected dependencies will require updating many files that rely on the global instances.

### Build / test / upload (practical commands)

- Build for device (board in `platformio.ini`):
  - platformio build: `pio run -e uno_r4_wifi`
  - upload: `pio run -e uno_r4_wifi -t upload`
- Run unit tests (native): platformio uses a `native` environment and Unity test framework. Run tests with:
  - `pio test -e native`
- Quick checks that matter to PR reviewers: lint/compile the `native` test env and the `uno_r4_wifi` build to catch include/compile errors early.

Notes: `platformio.ini` lists C++ deps used by the firmware (ArduinoJson, TinyGPSPlus, TFT_eSPI, rosserial_arduino, Unity). See `[env:*]` sections for details.

### Important protocols & message formats

- ROS2 bridge over Serial (`src/ROS2_Serial/ROS2_Serial.cpp`): JSON messages with a `sensor_type` field. Examples:
  - Incoming command (velocity):
    {"sensor_type":"cmd_vel","data":{"linear":{"x":0.10},"angular":{"z":0.00}}}
    The code reads `data.linear.x` and `data.angular.z` and calls `Motion::getInstance().setVelocity(...)`.
  - Outgoing IMU sample: {"sensor_type":"imu","data":{"ax":...,"ay":...,"az":...,"gx":...,"gy":...,"gz":...}}
  - Outgoing encoder sample: {"sensor_type":"encoder","data":{"left_wheel_ticks":...,"right_wheel_ticks":...}}
  - Outgoing GPS sample: {"sensor_type":"gps","data":{"latitude":...,"longitude":...}}

When modifying serial/JSON behavior, edit `ROS2_Serial.cpp` and respect the existing `deserializeJson`/`serializeJson` usage (ArduinoJson v7 style).

### Project-specific conventions & gotchas

- Singletons and globals: Many modules provide a global instance in their `.cpp`. Search for patterns like `SomeType someName;` at top-level. This affects initialization order and header inclusion cycles.
- File/directory names: a directory contains an ampersand (`objectavoidance&detection`) — be careful with shell globbing and scripts (quote paths).
- Serial port baud: `Serial.begin(9600);` in `main.cpp`. Serial-based integration tests and ROS2 bridge expect this setting unless explicitly changed.
- Platform targets: `uno_r4_wifi` (renesas-ra) is the primary target. The `native` environment is used only for unit tests. Avoid adding native-only code into firmware-only files.

### Files to read first (quick tour)

- `FollowBot/src/main.cpp` — basic setup/loop and Serial init
- `FollowBot/src/followbot_manager/FollowBotManager.cpp` — how the system is composed and lifecycle calls
- `FollowBot/src/ROS2_Serial/ROS2_Serial.cpp` — Serial JSON schema and update loop
- `FollowBot/src/motion/motion.h` & `.cpp` — Motion singleton, velocity/kinematics handling
- `FollowBot/src/followbot_client/FollowBotBluetooth.*` and `FollowBot/src/followbot_client/FollowBotClient.*` — external control client code paths
- `FollowBot/platformio.ini` — build/test targets and libs

### Editing guidance for AI agents (concrete rules)

- When adding a new subsystem, create a namespaced folder under `src/`, provide a `.h` and `.cpp`, and expose a single global instance (follow existing pattern) unless you also update `FollowBotManager` and all callers.
- Avoid changing the serial message topology in `ROS2_Serial` without also updating any Python frontends and the ROS2 bridge expecting JSON keys (`sensor_type`, `data.linear.x`, `data.angular.z`).
- If you must refactor globals into injected instances, update `FollowBotManager` to construct and pass the instances and update `main.cpp` ordering — unit tests (`native`) will catch link-time issues.

### Testing notes

- Unit tests live in `test/` and target the `native` environment (Unity). Tests include `MockArduino.h` — test code may depend on Arduino-compatible mocks.
- Run: `pio test -e native`. If adding tests, follow existing Unity-style tests.

---
If anything here is outdated or you want specific examples (e.g., typical JSON message payloads, Motor API calls, or UI layout config found in `followbot_ui/User_Setup.h`), tell me which area to expand and I will iterate.
