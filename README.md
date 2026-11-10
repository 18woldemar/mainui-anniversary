> [!NOTE]
> **This is the menu of [xash3d-anniversary](https://github.com/18woldemar/xash3d-anniversary)**, a fork of
> FWGS's [mainui_cpp](https://github.com/FWGS/mainui_cpp). It adds a gamepad menu in the WON style (switched on
> with the engine's `ui_gamepadui`), the render style and HD models switches, and the caption options. Report its
> bugs in [xash3d-anniversary](https://github.com/18woldemar/xash3d-anniversary/issues). Upstream's README follows.

# mainui_cpp

Main Menu UI for Xash3D FWGS engine, built entirely in C++.

Key features:
* Custom font renderer with multiple backends based on FreeType, stb_truetype and GDI on Windows
* Unicode and l10n support
* Completely self-contained, just clone this repo (with --recursive) and add to your mod
* Complex widget containers system, ability to put widget into containers into container
* Window system as a continuation of containers system
* Client-side menus (needs to be documented)

### Including mainui_cpp in your mod

1. Clone this repo: `git clone --recursive https://github.com/FWGS/mainui_cpp` to your mod source code tree.
2. If you use:
* CMake: invoke `add_subdirectory(mainui_cpp)` in your CMakeLists.txt file
* Waf/WAiFu: invoke `ctx.add_subdirectory('mainui_cpp')` in your wscript file
* Visual Studio: if you want to build it from your mod tree, include `vs2022/mainui_cpp.vcxproj` to your mod solution, or use `vs2022/mainui_cpp.sln`.
3. Place your built `menu.dll`/`libmenu.so` ALONGSIDE your `client.dll`/`client.so`.
4. Check that everything is working and happy hacking!

If you have any troubles setting this up, create an issue in https://github.com/FWGS/mainui_cpp/issues.

### Notes and restrictions

* mainui_cpp doesn't supports original Xash3D anymore. If it's possible, you can switch to Xash3D FWGS, otherwise you're on your own. I will accept patches to enable other Xash3D forks, but I won't support them on my own.

