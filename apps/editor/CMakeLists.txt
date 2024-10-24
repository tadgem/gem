cmake_minimum_required(VERSION 3.16)

add_executable(editor main.cpp editor_definitions.h editor_app.h editor_app.cpp "editor_definitions.h" "editor_app.cpp")

target_link_libraries(editor PRIVATE gle)
target_include_directories(editor PRIVATE ${GLE_INCLUDES})