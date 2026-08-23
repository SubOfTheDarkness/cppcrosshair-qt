# CppCrosshair ToolKit

An ultralight, high-performance crosshair toolkit for Linux (X11 & XWayland). It consists of two completely separate binaries: a heavy Qt6 pixel-art vector editor and an independent, standalone background overlay written in pure C++/X11 that consumes **0.00% CPU** and weighs only **~25 KB**.

## Key Features

* **Dual-Binary Architecture:** The heavy GUI editor runs only when you need to draw or manage processes, leaving an ultralight background overlay while you play.
* **Click-Through Window:** The overlay uses the native `XShape` extension to become completely invisible to mouse inputs. All clicks and camera movements pass directly into the game.
* **Full-Color Palette:** Powered by the XPM (X PixMap) format, allowing pixel-perfect multi-color crosshairs (e.g., bright cores with high-contrast outlines) so the crosshair never blends into the environment.
* **Interactive 32x32 Canvas:** Fluid pixel-art editor with continuous mouse-drag drawing, canvas loading on startup, and a clear guide line grid for perfect centering.
* **Hardware Offsets:** Built-in `X Offset` and `Y Offset` spinboxes to correct screen alignment values straight from the GUI interface.
* **Process Attach Logic:** The editor automatically detects if an overlay is already running via background process tracking, dynamically shifting the toggle button into control/termination mode.

## System Configuration
The project respects Linux standards and isolates user parameters cleanly. No files are written adjacent to system binaries. Everything is stored locally inside the user's home directory:
* **`~/.config/cppcrosshair/config.ini`** — Manages active paths, target assets, and pixel offset coordinates.
* **`~/.config/cppcrosshair/crosshair.xpm`** — Stores the customized 32x32 color matrix.

------------------------------

## Building and Installation

### System Requirements
* A modern compiler with **C++17** support (GCC 12/14 or Clang).
* **CMake** build tool (version 3.16+).
* **Qt 6** framework (Core, Gui, Widgets components for the editor module).
* **X11 development packages** (`libX11`, `libXext`, `libXpm`).

### 1. Generating .deb Package (For Debian / Ubuntu)
To generate a production-ready `.deb` package that resolves dependencies, registers system binaries, and deploys a desktop entry shortcut into the Cinnamon app menu:
```sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make package
```
Install the package using: `sudo apt install ./cppcrosshair-toolkit-<version>-amd64.deb`

### 2. Generating Native Arch Package
A local `PKGBUILD` script is ready in the repository root. To trigger an automatic, self-managed compilation and clean system deployment using the native `pacman` backend:
```sh
makepkg -si
```

------------------------------

## File Tree Organization
* `CMakeLists.txt` — Central multi-target build script.
* `config_parser.h` — Header-only isolated parser for user space INI data.
* `overlay_main.cpp` — Runtime source file for the 19 KB background X11 rendering asset.
* `editor_main.cpp` — Initialization entry point for the desktop frontend.
* `overlay_editor_window.h / .cpp / .ui` — Controller logic managing the layout matrix, palette routing, and process pgrep attachment tracking.
* `pixel_canvas.h / .cpp` — Custom promoted pixel-art widget managing manual painter layers, chess grid tiles, and coordinate calculations.
* `cppcrosshair.desktop` — Desktop shortcut configuration profile.
