cmake_minimum_required(VERSION 3.10)
project(AutoPartsFrontend)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/glfw)

file(GLOB IMGUI_SRC
    third_party/imgui/*.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(frontend
    main.cpp
    ${IMGUI_SRC}
)

target_include_directories(frontend PRIVATE
    third_party/imgui
    third_party/imgui/backends
    third_party/
    ../backend
    ../backend/third_party/json/include
    ../backend/third_party/cpp-httplib
)

if(APPLE)
    find_package(OpenGL REQUIRED)
    target_compile_definitions(frontend PRIVATE GL_SILENCE_DEPRECATION)
    target_link_libraries(frontend glfw OpenGL::GL)
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(frontend glfw OpenGL::GL)
endif()