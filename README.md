# Introduction

The purpose of this repository is to distribute the source files that are used to choose which OpenGL 3.3-compatible extensions should be allowed in [KalaWindow OpenGL binary](https://github.com/KalaKit/KalaWindow).

> The copy of the [official gl.xml file](https://registry.khronos.org/OpenGL/xml/gl.xml) has been placed to the *parser_in* folder.

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