cmake_minimum_required(VERSION 3.16)

add_executable(assets-test main.cpp)

target_link_libraries(assets-test PRIVATE gle)
target_include_directories(assets-test PRIVATE ${GLE_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})