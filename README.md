# Introduction

The purpose of this repository is to distribute the source files that are used to choose which Vulkan 1.2-compatible opt-in-only extensions should be allowed in [KalaWindow Vulkan binary](https://github.com/KalaKit/KalaWindow).

The official Vulkan API is not required to build or run the executable that creates the filtered files in *parser_out* because all required files are already placed inside *parser_in*.

> The copy of the [official vk.xml file](https://github.com/KhronosGroup/Vulkan-Docs/blob/main/xml/vk.xml) has been placed to the *parser_in* folder.

> The copies of the json files used for layers have been placed to the *parser_in* folder, each layer has its own json file.

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

---

# Filtering extensions

The filtering system for extensions is split into two passes.

## Extension pre-pass filter

...

## Extension post-pass filter

...

---

# Filtering layers

...
