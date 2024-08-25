cmake_minimum_required(VERSION 3.16)
project(gl-fun)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable ASSIMP dynamic linking")

add_subdirectory(third-party/sdl)
add_subdirectory(third-party/glew/build/cmake)
add_subdirectory(third-party/glm)
add_subdirectory(submodules/assimp)

set(GL_SRC 
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_opengl3.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/backends/imgui_impl_sdl2.cpp
        main.cpp
        gl.cpp
        gl.h
        shader.cpp
        shader.h
        texture.cpp
        texture.h
        model.cpp
        model.h
        shape.cpp
        shape.h
        vertex.cpp
        vertex.h
        utils.cpp
        utils.h
        stb_image.h
)

add_executable(demo ${GL_SRC})

set(GL_INCLUDES 
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/sdl/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/assimp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party/glm
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(demo PRIVATE glew_s SDL2-static glm)
target_include_directories(demo PRIVATE ${GL_INCLUDES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})