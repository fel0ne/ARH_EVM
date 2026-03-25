cmake_minimum_required(VERSION 3.10)
project(AutoPartsFrontend)

set(CMAKE_CXX_STANDARD 17)

# GLFW
add_subdirectory(third_party/glfw)

# ImGui source
file(GLOB IMGUI_SRC
    third_party/imgui/*.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# ✅ СНАЧАЛА создаем exe
add_executable(frontend
    main.cpp
    ${IMGUI_SRC}
)

# ✅ ПОТОМ include directories
target_include_directories(frontend PRIVATE
    third_party/imgui
    third_party/imgui/backends
    ../backend
    ../backend/third_party/json/include
    ../backend/third_party/cpp-httplib
)

# OpenGL
find_package(OpenGL REQUIRED)

# ✅ Линковка
target_link_libraries(frontend glfw OpenGL::GL)