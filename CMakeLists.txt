cmake_minimum_required(VERSION 3.15)
project(MachiningForceSimulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Find Packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# 2. Collect Sources (GLOB is okay for small projects, but manual is safer)
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"    # Added .c for glad if necessary
)

# 3. Create Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    # Often vcpkg puts imgui headers in a subfolder
)

# 5. Link Libraries
# Note: Check if your 'glad' target is glad::glad or just glad
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw 
    glad::glad 
    imgui::imgui 
    implot::implot 
    OpenGL::GL
)

# 6. Windows Specifics (Ensures OpenGL and Math link correctly)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Gdi32 User32 Shell32)
endif()