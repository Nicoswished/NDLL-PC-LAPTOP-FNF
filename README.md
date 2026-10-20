# sportedout-windows

**A Windows x64 native library for Haxe/CFFI Prime**

`sportedout-windows` is a native Windows library designed to provide window transparency, taskbar controls, system information, and other Windows desktop utilities to Haxe-based applications.

**Developed by Nicoswished**  
**Team:** Team Only 9.99$
**Mod By:** NitroLight

## 💎 Exclusive Release

**NitroLight Sported Out Team Only — $9.99 USD**

This mod is an exclusive release by **NitroLight Sported Out Team**. Access to the mod is priced at **$9.99 USD**.

Redistribution, resale, or reuse of the project without authorization from the creator is not permitted.

## Features

- **Color-key transparency** — Make a window transparent using a specified RGB color.
- **Window opacity** — Set window opacity using an alpha value from `0` to `255`.
- **Transparency modes** — Automatically detect the system configuration or force a specific mode.
- **System information** — Detect laptop/desktop status, available GPU adapters, and hybrid graphics configurations.
- **Taskbar control** — Hide or show the Windows taskbar, including secondary taskbars.
- **Window icon control** — Hide or restore the system menu icon.
- **Wallpaper access** — Retrieve the current desktop wallpaper path or change the wallpaper.
- **RAM detection** — Retrieve the system's installed or detected physical memory in MB.
- **Window visibility control** — Hide other visible application windows and restore them afterward.
- **Error logging** — Write diagnostic messages to `sportedout_ndll.log`.
- **Crash protection** — Use Windows Structured Exception Handling around selected operations.

## Requirements

- Windows 10 or Windows 11, 64-bit.
- [Haxe](https://haxe.org/)
- [HaxeLib](https://lib.haxe.org/)
- `hxcpp` 4.3.x or a compatible version.
- A compatible C++ build toolchain, such as Microsoft Visual Studio Build Tools.
- Windows SDK libraries, including `user32.lib` and `dxgi.lib`.

## Project structure

```text
sportedout-windows/
├── build.bat
├── Build.xml
├── src/
│   └── transparency.cpp
└── ndll/
    └── Windows64/
        └── ndll-sportedout-windows.ndll
```

The compiled NDLL is intended to be placed in `ndll/Windows64/`.

## Building

1. Install Haxe and HaxeLib.
2. Install `hxcpp`:

   ```bash
   haxelib install hxcpp
   ```

3. Make sure your C++ compiler and Windows SDK are installed and configured.
4. Open a terminal in the project directory.
5. Run:

   ```bat
   build.bat
   ```

The build script executes:

```bat
haxelib run hxcpp Build.xml -Dwindows -DHXCPP_M64
pause
```

This configures an x64 Windows build through `hxcpp`.

After compilation, verify that the generated NDLL is available in the expected output directory.

## Available native functions

| Function | Description |
|---|---|
| `set_transparent(r, g, b)` | Makes the specified RGB color transparent. |
| `set_alpha(alpha)` | Sets the window opacity, from `0` to `255`. |
| `remove_transparent()` | Removes the layered-window style used for transparency. |
| `set_transparent_mode(mode)` | Forces a transparency mode or resets to automatic selection with `0`. |
| `get_system_info()` | Returns a string describing the detected system and GPU adapters. |
| `hide_taskbar()` | Hides the Windows taskbar. |
| `show_taskbar()` | Shows the Windows taskbar. |
| `hide_window_icon()` | Removes the system menu style from the selected window. |
| `show_window_icon()` | Restores the system menu style. |
| `get_wallpaper()` | Returns the desktop wallpaper path. |
| `change_wallpaper(path)` | Attempts to change the desktop wallpaper. |
| `get_ram()` | Returns detected physical memory in MB. |
| `hide_windows(arg)` | Hides other eligible visible application windows. |
| `show_windows(arg)` | Restores windows hidden by the library. |

**Note:** The native functions must be declared and called using signatures compatible with Haxe/CFFI Prime. The exact Haxe declarations depend on how the library is integrated into the host project.

## Transparency modes

The library supports automatic mode selection and forced modes.

| Mode | Behavior |
|---|---|
| `0` | Automatic detection. |
| `1` | Standard mode. |
| `2` | Laptop mode. |
| `3` | Hybrid graphics mode. |

In automatic mode, the library checks the system's power status and graphics adapters to choose a mode. These modes control internal refresh behavior; they do not guarantee compatibility with every graphics configuration.

## Logging and troubleshooting

The library writes diagnostic messages to:

```text
sportedout_ndll.log
```

If transparency does not work:

1. Confirm that the target application has a valid top-level window.
2. Check that the NDLL was built for Windows x64.
3. Verify that the required native libraries and runtime dependencies are available.
4. Inspect the log for window detection or Windows API errors.
5. Confirm that the function signatures match the Haxe/CFFI Prime declarations.

## Important notes

- The library targets Windows and is not intended for Linux or macOS.
- Window operations target the current process's application window where applicable.
- Hiding other windows can temporarily make desktop applications inaccessible until they are restored.
- Wallpaper changes and taskbar operations affect the Windows desktop environment.
- Crash protection does not guarantee that every possible failure will be caught.
- The build configuration and generated output paths should be verified against your installed `hxcpp` toolchain.
- This release is associated with **NitroLight Sported Out Team** and is priced at **$9.99 USD**.

## License and distribution

This project is an exclusive release by **NitroLight Sported Out Team**.

**Team** Only 9.99$

Unauthorized redistribution, resale, or reuse is prohibited unless permission is granted by the creator, **Nicoswished**.

For inquiries regarding access, licensing, or permissions, contact **Nicoswished on Discord**.
