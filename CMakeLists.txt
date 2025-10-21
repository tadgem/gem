cmake_minimum_required(VERSION 3.16)
project(gem-engine)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# built in options
set(CMAKE_CXX_STANDARD 17)

# development options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# library options
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable ASSIMP dynamic linking")
set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC TRUE CACHE INTERNAL TRUE)
set(SDL_ASAN ON CACHE INTERNAL "")
set(CMAKE_SUPPRESS_REGENERATION true)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable SDL Entrypoint")
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Disable SDL Install")
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable SDL Uninstall")
set(OpenGL_GL_PREFERENCE "GLVND")
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/ CACHE INTERNAL "")
set(SUBMODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/ CACHE INTERNAL "")
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(SDL_THREADS_ENABLED_BY_DEFAULT ON)
set(SDL_THREADS ON)

if(NOT MSVC)
    message("-- Disabling DirectX shader compilation")
    set(SDLSHADERCROSS_DXC OFF CACHE BOOL "Disable DirectX shader compilation on linux")
endif()

add_subdirectory(submodules/cpptrace)
add_subdirectory(submodules/spdlog)
add_subdirectory(submodules/efsw)
add_subdirectory(third-party/sdl)
add_subdirectory(third-party/tracy)
add_subdirectory(third-party/tracy/profiler)
add_subdirectory(third-party/glew/build/cmake)
add_subdirectory(third-party/glm)
add_subdirectory(third-party/assimp)
add_subdirectory(gem)
add_subdirectory(tests)
add_subdirectory(apps/assets_test)
add_subdirectory(apps/renderer_demo)
add_subdirectory(apps/gi_fixup)
add_subdirectory(apps/editor)
add_subdirectory(apps/sdl_gpu_test)
