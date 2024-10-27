cmake_minimum_required(VERSION 3.10)

# Project Name
project(VoxelVolume)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)

# Define configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available build types" FORCE)

# Set include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/include
    ${CMAKE_SOURCE_DIR}/Dependencies/glm
    ${CMAKE_SOURCE_DIR}/Dependencies
)

# Set library directories
link_directories(
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/
)

# Add preprocessor definitions and compiler options
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DGLEW_STATIC)  # Important for static GLEW library
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_DEBUG)
    else() 
        add_definitions(-DNDEBUG)
    endif()
endif()

# Add the source files
file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")


# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link the libraries
target_link_libraries(${PROJECT_NAME} 
    ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib 
    ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib 
    opengl32
    gdi32       # Required for GLFW (Windows GUI)
    user32      # Required for GLFW (Windows UI)
    kernel32    # Required for Windows system functions
)

# Set specific compiler and linker flags for different configurations
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

message(STATUS "Using GLFW from: ${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2022/glfw3.lib")
message(STATUS "Using GLEW from: ${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32s.lib")
message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX DEBUG FLAGS: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX RELEASE FLAGS: ${CMAKE_CXX_FLAGS_RELEASE}")

# add_definitions(-DGLEW_STATIC)
# add_definitions(-D_CRT_SECURE_NO_WARNINGS)





