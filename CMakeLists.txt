cmake_minimum_required(VERSION 3.10)

# Project Name
project(VoxelVolume)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) # Enforce the standard

# Define configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available build types" FORCE)

# --- Common Source Files (used by all platforms) ---
file(GLOB_RECURSE APP_SOURCES
    "src/*.cpp"
    "main.cpp" # Assuming main.cpp is in the project root alongside CMakeLists.txt
)
file(GLOB_RECURSE APP_HEADERS "src/*.h")

set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_demo.cpp # As you noted, for demo purposes
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends/imgui_impl_opengl3.cpp
)

# Add the executable
add_executable(${PROJECT_NAME}
    ${APP_SOURCES}
    ${APP_HEADERS} # Headers here help IDEs, not strictly needed for compilation itself
    ${IMGUI_SOURCES}
)

# --- Common Include Directories for project-local dependencies ---
# These are for dependencies you've included directly in your project structure (like GLM, ImGui)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Dependencies/glm
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/backends
    ${CMAKE_SOURCE_DIR}/Dependencies # General dependencies folder
    ${CMAKE_CURRENT_SOURCE_DIR}/src # Assuming your project sources are in 'src'
)

# --- Platform Specific Configuration ---
if(WIN32)
    message(STATUS "Configuring for Windows")

    # Set MSVC runtime library
    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
        # GLEW_STATIC is important if you are linking the static GLEW library on Windows
        add_compile_definitions(GLEW_STATIC)
    endif()

    # Include directories for Windows-specific vendored libraries
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/include
        ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/include
    )

    # Link libraries for Windows
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib
        ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib
        opengl32
        gdi32      # Required for GLFW (Windows GUI)
        user32     # Required for GLFW (Windows UI)
        kernel32   # Required for Windows system functions
    )

elseif(APPLE)
    message(STATUS "Configuring for macOS")

    # Suppress OpenGL deprecation warnings on macOS
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)

    # Handle __debugbreak for Clang on macOS
    # Option 1: Define it to __builtin_trap() or raise(SIGTRAP)
    # Option 2: Or make it a no-op if you prefer not to trap during GLCall errors for now
    # target_compile_definitions(${PROJECT_NAME} PRIVATE "__debugbreak()=")

    # Find packages (GLFW, GLEW) - Homebrew should make them findable
    # Hint Homebrew's prefix paths for CMake if it has trouble.
    # For Apple Silicon: /opt/homebrew, for Intel: /usr/local
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")

    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)

    # Include directories from Homebrew packages
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${GLFW_INCLUDE_DIRS}  # Provided by find_package(glfw3)
        ${GLEW_INCLUDE_DIRS}  # Provided by find_package(GLEW)
    )

    # Link libraries for macOS
    # find_package for glfw3 typically provides an imported target `glfw` or `GLFW::glfw`
    # find_package for GLEW typically provides an imported target `GLEW::glew` or `GLEW::GLEW`
    # If not, use ${GLFW_LIBRARIES} and ${GLEW_LIBRARIES}
    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw # Or GLFW::glfw if available
        GLEW::GLEW # Or GLEW::glew or ${GLEW_LIBRARIES} if the target isn't created
        "-framework OpenGL"    # Apple's OpenGL framework
        "-framework Cocoa"     # For windowing and events
        "-framework IOKit"     # For hardware interaction (often a GLFW dependency)
        "-framework CoreFoundation"
        "-framework CoreVideo" # Sometimes needed by GLFW
    )
    # Note: GLEW_STATIC is generally NOT used on macOS with Homebrew as it provides dynamic libs.

else()
    message(FATAL_ERROR "Unsupported platform. This CMakeLists.txt is configured for Windows and macOS.")
endif()


# --- Common Target Properties (Output directories) ---
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

# --- Common Status Messages ---
message(STATUS "Project source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Project binary dir: ${CMAKE_BINARY_DIR}")

if(WIN32)
    message(STATUS "Using GLFW (vendored) from: ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib")
    message(STATUS "Using GLEW (vendored) from: ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib")
endif()

message(STATUS "CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX DEBUG FLAGS: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX RELEASE FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")