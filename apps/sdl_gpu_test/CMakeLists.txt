cmake_minimum_required(VERSION 3.16)

add_executable(sdl-gpu-test main.cpp)

target_link_libraries(sdl-gpu-test PRIVATE gem)
target_include_directories(sdl-gpu-test PRIVATE ${GEM_INCLUDES})
