cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

add_executable(ecs-demo main.cpp)

target_link_libraries(ecs-demo PRIVATE gle)
target_include_directories(ecs-demo PRIVATE ${GLE_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})