set(GL_SRC 
        ${THIRD_PARTY_DIR}/imgui/imgui_draw.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui_tables.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui_widgets.cpp
        ${THIRD_PARTY_DIR}/imgui/imgui.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp
        ${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
        ${THIRD_PARTY_DIR}/soil/src/image_dxt.c
        ${THIRD_PARTY_DIR}/soil/src/image_helper.c
        ${THIRD_PARTY_DIR}/soil/src/soil.c
        ${THIRD_PARTY_DIR}/soil/include/SOIL/SOIL.h
        ${THIRD_PARTY_DIR}/soil/src/stb_image_aug.c
        ${SUBMODULE_DIR}/im3d/im3d.cpp
        ${SUBMODULE_DIR}/im3d/im3d_config.h
        ${SUBMODULE_DIR}/im3d/im3d_math.h
        ${SUBMODULE_DIR}/im3d/im3d.h
        ${CMAKE_CURRENT_SOURCE_DIR}/gl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/input.h
        ${CMAKE_CURRENT_SOURCE_DIR}/input.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/material.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/material.h
        ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/camera.h
        ${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/texture.h
        ${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shape.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/shape.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vertex.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vertex.h
        ${CMAKE_CURRENT_SOURCE_DIR}/im3d_gl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/im3d_gl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/voxelisation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/voxelisation.h
        # third party sources
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/json.hpp
)

add_library(gle STATIC ${GL_SRC})

set(GLE_INCLUDES 
        ${THIRD_PARTY_DIR}/glew/include
        ${THIRD_PARTY_DIR}/sdl/include
        ${THIRD_PARTY_DIR}/imgui
        ${THIRD_PARTY_DIR}/assimp/include
        ${THIRD_PARTY_DIR}/imgui/backends
        ${THIRD_PARTY_DIR}/glm
        ${THIRD_PARTY_DIR}/soil/include
        ${SUBMODULE_DIR}/im3d
        ${CMAKE_CURRENT_SOURCE_DIR}
CACHE INTERNAL "")

target_link_libraries(gle PRIVATE glew_s SDL2-static glm assimp)
target_include_directories(gle PUBLIC ${GLE_INCLUDES})