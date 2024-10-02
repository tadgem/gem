cmake_minimum_required(VERSION 3.16)

add_executable(gi-demo main.cpp)

target_link_libraries(gi-demo PRIVATE gle)
target_include_directories(gi-demo PRIVATE ${GLE_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})