cmake_minimum_required(VERSION 3.16)

add_executable(renderer-demo main.cpp)

target_link_libraries(renderer-demo PRIVATE gle)
target_include_directories(renderer-demo PRIVATE ${GLE_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})