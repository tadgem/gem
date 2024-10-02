cmake_minimum_required(VERSION 3.16)

set(AGPU_SRC 
    agpu.h
    agpu.cpp
)

add_library(agpu STATIC ${AGPU_SRC})

set(AGPU_INCLUDES 
        ${CMAKE_CURRENT_SOURCE_DIR}
CACHE INTERNAL "")

target_link_libraries(agpu PRIVATE glew_s SDL2-static glm)
target_include_directories(gle PUBLIC ${AGPU_INCLUDES})