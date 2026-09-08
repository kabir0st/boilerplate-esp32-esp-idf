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

## Why `src/` and not `main/`

ESP-IDF gives special treatment to a component directory named `main`. It is registered
automatically, and it receives every common component as an implicit dependency, so a
`main/` project can include almost any IDF header without declaring anything.

This project puts the application in `src/` instead, which costs two small pieces of
explicitness:

1. The root [CMakeLists.txt](CMakeLists.txt) must add the directory with
   `EXTRA_COMPONENT_DIRS`, and that must come **before** the `project()` call.
2. The component must declare its own dependencies in
   [src/CMakeLists.txt](src/CMakeLists.txt) under `REQUIRES`.

The second point is the one that will bite you. **Using a new IDF API means adding its
component to `REQUIRES`.** GPIO needs `esp_driver_gpio`, which is why it is already listed.
Non-volatile storage needs `nvs_flash`, Wi-Fi needs `esp_wifi`, and so on. FreeRTOS and
`esp_log` are common requirements and come in implicitly, which is why `main.cpp` includes
them without any declaration.

If a header will not resolve and the linker complains about a missing symbol from an IDF
component, a missing `REQUIRES` entry is the first thing to check.

## Using this as a template

1. Copy the directory to its new home, then delete `build/`, `sdkconfig` and
   `sdkconfig.old`. Those are all generated and all target-specific.
2. Rename the project in the last line of [CMakeLists.txt](CMakeLists.txt). That name
   becomes the ELF and binary filename.
3. Fix the machine-specific paths listed in the table below.
4. Run `idf.py set-target esp32`, substituting your chip. This regenerates `sdkconfig` from
   scratch, so do it before the first build.
5. Edit [src/main.cpp](src/main.cpp). New source files go into `SRCS` in
   [src/CMakeLists.txt](src/CMakeLists.txt), and new IDF APIs go into `REQUIRES`.

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

## Machine-specific settings

Three files contain absolute paths tied to this machine and this user. Change them first on
any new machine.

| File | Setting | Current value |
| --- | --- | --- |
| `.vscode/settings.json` | `idf.currentSetup` | `/home/kabir0st/.espressif/v6.1/esp-idf` |
| `.vscode/tasks.json` | activation script, in every task | `/home/kabir0st/.espressif/tools/activate_idf_v6.1.sh` |
| `.vscode/c_cpp_properties.json` | `compilerPath` and `includePath` | under `/home/kabir0st/.espressif/` |

## VS Code setup notes

The reasoning behind the editor configuration, so that it survives being copied:

- **Extension version.** ESP-IDF extension v2.2.0 dropped `idf.espIdfPath` and
  `idf.toolsPath`. It now reads the Installation Manager manifest, so the install is
  selected with `idf.currentSetup`. The `idf.eimIdfJsonPath` override is
  application-scoped and cannot be set in workspace settings.
- **Tasks run through bash.** The shell here is zsh, and Espressif ships only `.sh` and
  `.fish` activation scripts. Each task therefore sets its shell to `/bin/bash` explicitly.
- **PlatformIO is discouraged.** It claims ESP32 projects and supplies its own IntelliSense
  provider, and it fights cpptools over `c_cpp_properties.json`. It is listed under
  `unwantedRecommendations` for that reason.
- **IntelliSense comes from the compile database.** `build/compile_commands.json` is the
  real source of include paths, defines and language flags, and it is regenerated on every
  build. That means a **new component is only visible to IntelliSense after a build**.
- **No `cStandard` or `cppStandard`.** The project builds as gnu23 and gnu++26, and cpptools
  has no setting for the latter. Leaving both out lets the compile database win.

## Troubleshooting

**`idf.py: command not found`.** The activation script was not sourced in this shell. It has
to be sourced again in every new terminal.

**IDF headers show as unresolved in the editor.** Build once so that
`build/compile_commands.json` exists and is current, then reload the VS Code window.

**`Permission denied: /dev/ttyUSB0`.** Add your user to the `dialout` group with
`sudo usermod -aG dialout $USER`, then log out and back in.

**Configuration looks wrong after changing target.** Run `idf.py fullclean` and build again.
If it persists, delete `sdkconfig` and rerun `idf.py set-target <chip>`.

## A note on `sdkconfig`

`sdkconfig` is tracked here, because a reference project is more useful when its exact
configuration is pinned. It is also generated, target-specific, and large, so it produces
noisy diffs. A stricter project ignores `sdkconfig` and commits a small
`sdkconfig.defaults` holding only the settings that were changed deliberately.
