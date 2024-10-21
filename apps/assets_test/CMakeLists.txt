cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

add_executable(assets-test main.cpp)

target_link_libraries(assets-test PRIVATE gle cpptrace::cpptrace)
target_include_directories(assets-test PRIVATE ${GLE_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  add_custom_command(
    TARGET assets-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:cpptrace::cpptrace>
    $<TARGET_FILE_DIR:assets-test>
  )
endif()