cmake_minimum_required(VERSION 3.16)
project(gl-fun)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable ASSIMP dynamic linking")
set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC TRUE CACHE INTERNAL TRUE)
set(CMAKE_SUPPRESS_REGENERATION true)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable SDL Entrypoint")
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Disable SDL Install")
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable SDL Uninstall")

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ CACHE INTERNAL "")
set(SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/ CACHE INTERNAL "")

add_subdirectory(third-party/sdl)
add_subdirectory(third-party/glew/build/cmake)
add_subdirectory(third-party/glm)
add_subdirectory(third-party/assimp)
add_subdirectory(gle)

add_executable(demo main.cpp)

target_link_libraries(demo PRIVATE gle)
target_include_directories(demo PRIVATE ${GLE_INCLUDES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})