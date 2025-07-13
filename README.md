# Introduction

The purpose of this repository is to distribute the source files that are used to choose which OpenGL 3.3-compatible opt-in extensions and wgl extensions should be allowed in [KalaWindow OpenGL binary](https://github.com/KalaKit/KalaWindow).

> The copies for the [gl.xml file](https://github.com/KhronosGroup/OpenGL-Registry/blob/main/xml/gl.xml) and the [windows wgl.xml](https://github.com/KhronosGroup/OpenGL-Registry/blob/main/xml/wgl.xml) and the [linux glx.xml] files have been placed to the *parser_in* folder.

---

# Compilation

The executable that creates the filters has these requirements:
- CMake 3.29.2 or newer
- Ninja
- Visual Studio 2022 IDE or Compilation tools

Run *build_all_windows.bat* to compile from source code.

---

# Running the executable

Once the executable has been compiled simply run it in *out/build/debug/KalaTool.exe* or *out/build/release/KalaTool.exe* and press Enter to close the executable once it has finished.