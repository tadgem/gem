cmake_minimum_required(VERSION 3.16)

add_executable(gi_fixup main.cpp)

target_link_libraries(gi_fixup PRIVATE gem)
target_include_directories(gi_fixup PRIVATE ${GEM_INCLUDES})

file(COPY ${ASSETS_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})