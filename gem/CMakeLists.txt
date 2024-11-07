file(GLOB_RECURSE GEM_SRC_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" src/*.cpp)
file(GLOB_RECURSE GEM_HEADER_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" include/*.h)
file(GLOB_RECURSE GEM_HEADERPP_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" include/*.hpp)


set(GEM_SRC
        ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui.cpp
        ${THIRD_PARTY_DIR}/imgui/ImFileDialog.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
        ${THIRD_PARTY_DIR}/im3d/im3d.cpp
        ${THIRD_PARTY_DIR}/im3d/im3d_config.h
        ${THIRD_PARTY_DIR}/im3d/im3d_math.h
        ${THIRD_PARTY_DIR}/im3d/im3d.h
        ${GEM_SRC_FILES}
        ${GEM_HEADER_FILES}
        ${GEM_HEADERPP_FILES}
)

set(CMAKE_CXX_STANDARD 17)

add_library(gem STATIC ${GEM_SRC})

set(GEM_INCLUDES
        ${THIRD_PARTY_DIR}/glew/include
        ${THIRD_PARTY_DIR}/sdl/include
        ${THIRD_PARTY_DIR}/imgui
        ${THIRD_PARTY_DIR}/assimp/include
        ${THIRD_PARTY_DIR}/imgui/backends
        ${THIRD_PARTY_DIR}/glm
        ${THIRD_PARTY_DIR}/gli/gli
        ${THIRD_PARTY_DIR}/entt
        ${THIRD_PARTY_DIR}/im3d
        ${THIRD_PARTY_DIR}/tracy/public
        ${THIRD_PARTY_DIR}
        ${SUBMODULE_DIR}/spdlog/include
        ${SUBMODULE_DIR}/cpptrace/include
        ${SUBMODULE_DIR}/efsw/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        CACHE INTERNAL "")

target_link_libraries(gem PRIVATE efsw glew_s SDL2-static glm assimp cpptrace::cpptrace spdlog TracyClient)
target_include_directories(gem PUBLIC ${GEM_INCLUDES})