# CppCrosshair ToolKit

An ultralight, high-performance crosshair toolkit for Linux (X11 & XWayland). It consists of two completely separate binaries: a heavy Qt6 pixel-art vector editor and an independent, standalone background overlay written in pure C++/X11 that consumes **0.00% CPU** and weighs only **~25 KB**.

## Key Features

* **Dual-Binary Architecture:** The heavy GUI editor runs only when you need to draw or manage processes, leaving an ultralight background overlay while you play.
* **KDE Plasma 6 Widget Plugin:** An independent, binary-driven native C++ desktop widget (`.so`) that embeds a complete remote controller straight into your system taskbar or desktop layout.
* **Click-Through Window:** The overlay uses the native `XShape` extension to become completely invisible to mouse inputs. All clicks and camera movements pass directly into the game.
* **Full-Color Palette:** Powered by the XPM (X PixMap) format, allowing pixel-perfect multi-color crosshairs (e.g., bright cores with high-contrast outlines) so the crosshair never blends into the environment.
* **Interactive 32x32 Canvas:** Fluid pixel-art editor with continuous mouse-drag drawing, canvas loading on startup, and a clear guide line grid for perfect centering.
* **Hardware Offsets:** Built-in `X Offset` and `Y Offset` spinboxes to correct screen alignment values straight from the GUI or Plasma widget interface.
* **Process Attach Logic:** The editor and the desktop widget automatically detect if an overlay is already running via background process tracking, dynamically shifting the toggle button into control/termination mode.

...

## Building and Installation

### System Requirements
* A modern compiler with **C++17** support (GCC 12/14 or Clang).
* **CMake** build tool (version 3.16+).
* **Qt 6** framework (`Core`, `Gui`, `Widgets` components for the editor; `Qml` for the widget plugin).
* **X11 development packages** (`libX11`, `libXext`, `libXpm`).

### Compiling Standalone Core Applications (Default)
By default, compiling the project only builds the independent core binaries (the standalone GUI editor and the background X11 daemon). No KDE Plasma dependencies or extra package overhead will be pulled into your system layout:
```sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

### Compiling with KDE Plasma 6 Desktop Widget
If you are running the KDE Plasma 6 desktop environment, pass the explicit `-DBUILD_PLASMA_WIDGET=ON` flag to trigger an isolated native compilation of the binary `.so` module and deploy the corresponding system configuration layout:
```sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_PLASMA_WIDGET=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make
```

Once compilation succeeds, you have granular control over exactly what parts of the ecosystem you wish to deploy via **CMake Component Target Allocation**:

#### 1. Deploy ONLY the Core Applications (To System Global `/usr/bin/`)
```sh
sudo cmake --install . --component app
```

#### 2. Deploy ONLY the Desktop Widget Plugin (To Local User Directory `~/.local/`)
This does not require root privileges or `sudo`. It compiles the C++ backend directly into a sandboxed native QML library module, storing the layout locally inside your individual user account configurations:
```sh
cmake --install . --component widget
```

#### 3. Deploy Everything Simultaneously
```sh
sudo make install
```

### Generating Distribution Packages

#### 1. Generating .deb Package (For Debian / Ubuntu)
To generate a clean production-ready `.deb` package that isolates system binaries and registers an executable launcher shortcut:
```sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cpack
```
*Note: The widget plugin installation structure is entirely isolated from the generation pipeline. The final `.deb` package will only pack standalone system binaries without bloating the layout for non-KDE environments.*

#### 2. Generating Native Arch Package
A local `PKGBUILD` script is ready in the repository root. To trigger an automatic, self-managed compilation and clean system deployment using the native `pacman` backend:
```sh
makepkg -si
```

------------------------------

## File Tree Organization
* `CMakeLists.txt` — Central multi-target build script with modular component allocation flags.
* `config_parser.h` — Header-only isolated parser for user space INI data.
* `overlay_main.cpp` — Runtime source file for the 19 KB background X11 rendering asset.
* `editor_main.cpp` — Initialization entry point for the desktop frontend.
* `overlay_editor_window.h / .cpp / .ui` — Controller logic managing the layout matrix, palette routing, and process pgrep attachment tracking.
* `pixel_canvas.h / .cpp` — Custom promoted pixel-art widget managing manual painter layers, chess grid tiles, and coordinate calculations.
* `cppcrosshair.desktop` — Desktop shortcut configuration profile.
* `plasma-widget/` — Isolated desktop extension environment.
    * `CMakeLists.txt` — QML module generator leveraging native C++ `qt_add_qml_module` compilation layers.
    * `metadata.json` — System manifest scheme defining Plasma 6 package constraints.
    * `crosshair_manager.h / .cpp` — Thread-safe X11 process manager running independent `QTimer` background state scanning loops.
    * `contents/ui/main.qml` — Highly interactive reactive panel UI dispatching control bindings to the binary plugin.
    * `contents/ui/BackendWrapper.qml` — Sandboxed QML module wrapper ensuring type-safe, race-condition-free runtime loading.

## Wayland Compatibility

Since this overlay is built natively on top of pure X11/Xlib (`override_redirect`), it runs via **XWayland** on modern Wayland-based desktops. By default, the window manager might hide the crosshair beneath native Wayland windows or games.

### How to fix it:

1. **Recommended (Best Stability & Performance):** Switch your desktop session from **Wayland** to **X11** on your system login screen (Log Out -> Select X11 -> Log In). This allows the overlay to function flawlessly as intended.
2. **Alternative (Window Rules Workaround):** If you must stay on Wayland, force the system to render the crosshair on top:
   * Open **System Settings** -> **Window Management** -> **Window Rules**.
   * Click **Add New Rule...** and set the Window class to `crosshair_overlay`.
   * Add the **Keep above** property, set it to **Force**, and select **Yes**.
   * Apply changes.
