cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    main.cpp
    chessengine.cpp
    game.cpp
    game.h
    movetables.cpp
    glad/src/gl.c
    imgui-master/imgui.cpp
    imgui-master/imgui_draw.cpp
    imgui-master/imgui_widgets.cpp
    imgui-master/imgui_tables.cpp
    imgui-master/backends/imgui_impl_glfw.cpp
    imgui-master/backends/imgui_impl_opengl3.cpp
        textures.cpp
        stb_image.h
        textures.h
)

add_executable(ChessEngine ${SOURCES})

target_include_directories(ChessEngine PRIVATE
        glad/include
        imgui-master
        imgui-master/backends
        glfw-3.4.bin.WIN64/include
        ${CMAKE_CURRENT_SOURCE_DIR}  # for chessengine.h
)

target_link_directories(ChessEngine PRIVATE glfw-3.4.bin.WIN64/lib-mingw-w64)
target_link_libraries(ChessEngine PRIVATE glfw3 opengl32)