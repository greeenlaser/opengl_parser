cmake_minimum_required(VERSION 3.29.2)

project(KalaTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC runtime handling
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# === Source ===
add_executable(KalaTool main.cpp)

target_compile_definitions(KalaTool PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# (Optional) Set executable output name
set_target_properties(KalaTool PROPERTIES OUTPUT_NAME "KalaTool")
