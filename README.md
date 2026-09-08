# blink-led

An ESP-IDF v6.1 blink application for the ESP32, written in C++, that doubles as a project
template for other ESP apps. Two LEDs are driven from `app_main`: a status LED on GPIO 33
that toggles every 500 ms, and a flash LED on GPIO 4 that toggles every fourth tick, so
every 2 seconds. The code itself is trivial on purpose. The value of this project is the
build layout and the editor configuration around it.

## Prerequisites

ESP-IDF v6.1, installed through the Espressif Installation Manager. On this machine the
framework lives at `~/.espressif/v6.1/esp-idf` and the activation script at
`~/.espressif/tools/activate_idf_v6.1.sh`. The manager records every install it knows about
in `~/.espressif/tools/eim_idf.json`, which is what the VS Code extension reads.

You also need a USB serial connection to the board and permission to open it. See
Troubleshooting if the port is denied.

## Build, flash, monitor

Every shell session starts by sourcing the activation script. It puts `idf.py` and the
cross toolchain on `PATH`.

```bash
source ~/.espressif/tools/activate_idf_v6.1.sh
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Press `Ctrl-]` to leave the serial monitor. Other commands you will want are
`idf.py menuconfig` to change configuration, `idf.py size` for a flash and RAM breakdown,
and `idf.py fullclean` to delete the build directory.

In VS Code the same commands are wired up as tasks, so `Ctrl-Shift-B` builds and the rest
are under Run Task. The tasks source the activation script themselves.

## Project layout

```
CMakeLists.txt      Root project file. Registers src/ as a component, names the project.
src/
  CMakeLists.txt    Component registration: source list, include dirs, dependencies.
  main.cpp          app_main lives here.
sdkconfig           Generated configuration. Do not hand-edit; use idf.py menuconfig.
build/              Generated artifacts. Not tracked.
.vscode/            Editor, task, IntelliSense and debug configuration.
```

## Targeting a different ESP chip

The project is currently set to `esp32`. Moving to another part takes more than
`set-target`, because a few settings name the ESP32 explicitly.

- `idf.py set-target esp32s3` (or `esp32c3`, `esp32c6`, and so on) regenerates `sdkconfig`
  completely. Any configuration you changed by hand is lost, which is one reason to keep
  deliberate settings in a `sdkconfig.defaults` file instead.
- The RISC-V parts, meaning the C3, C6 and H2 among others, use a different toolchain.
  Change `compilerPath` in [.vscode/c_cpp_properties.json](.vscode/c_cpp_properties.json)
  from `xtensa-esp32-elf-g++` to `riscv32-esp-elf-g++`. The Xtensa parts, S2 and S3, keep
  the Xtensa toolchain but under a different triple.
- `idf.openOcdConfigs` in [.vscode/settings.json](.vscode/settings.json) points at ESP32
  OpenOCD configs and only matters if you attach a JTAG probe.
- The `ESP-IDF: set-target esp32` task in [.vscode/tasks.json](.vscode/tasks.json) hardcodes
  the chip name.
- Pin assignments in `main.cpp` are board-specific. GPIO 33 and GPIO 4 are valid outputs on
  this ESP32 board and may not exist, or may be strapping pins, elsewhere. Check the
  schematic.

## A note on `sdkconfig`

`sdkconfig` is tracked here, because a reference project is more useful when its exact
configuration is pinned. It is also generated, target-specific, and large, so it produces
noisy diffs. A stricter project ignores `sdkconfig` and commits a small
`sdkconfig.defaults` holding only the settings that were changed deliberately.
