cmake_minimum_required(VERSION 3.16)

add_executable(agpu-demo main.cpp)

target_link_libraries(agpu-demo PRIVATE agpu)
target_include_directories(agpu-demo PRIVATE ${AGPU_INCLUDES})
