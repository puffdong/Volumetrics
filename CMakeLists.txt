cmake_minimum_required(VERSION 3.10)

# Project Name
project(VoxelVolume)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) # Enforce the standard

# Define configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available build types" FORCE)

# Set MSVC runtime library (helps with LNK4098 warnings)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/glm
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui      # For imgui.h, etc.
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends # For imgui_impl_glfw.h, imgui_impl_opengl3.h
    ${CMAKE_SOURCE_DIR}/Dependencies # General dependencies
    ${CMAKE_CURRENT_SOURCE_DIR}/src # Assuming your project sources are in 'src'
)

# Link directories (generally less preferred than providing full paths in target_link_libraries)
# link_directories(
#     ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/
#     ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/
# )

# Add preprocessor definitions and compiler options
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(GLEW_STATIC) # Important for static GLEW library
    # CMake handles _DEBUG/NDEBUG automatically based on build type (Debug/Release)
endif()

# Add the application source files
# Assuming your main.cpp and other project files are in a 'src' directory
# If main.cpp is in the root, adjust accordingly.
file(GLOB_RECURSE APP_SOURCES "src/*.cpp" "main.cpp") # Include main.cpp if it's in the root
file(GLOB_RECURSE APP_HEADERS "src/*.h")


# Add ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_demo.cpp # REMOVE THIS LATER BECAUSE ITS JUST A DEMO BRO
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

# Add the executable
add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${IMGUI_SOURCES}
)

# Link the libraries
target_link_libraries(${PROJECT_NAME} PRIVATE # Use PRIVATE or PUBLIC as appropriate
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib
    opengl32
    gdi32      # Required for GLFW (Windows GUI)
    user32     # Required for GLFW (Windows UI)
    kernel32   # Required for Windows system functions
)

# Set specific compiler and linker flags for different configurations
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

message(STATUS "Project source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Project binary dir: ${CMAKE_BINARY_DIR}")
message(STATUS "Using GLFW from: ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib")
message(STATUS "Using GLEW from: ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib")
message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX DEBUG FLAGS: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX RELEASE FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")