cmake_minimum_required(VERSION 3.30)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(ChessEngine main.cpp
        chessengine.cpp
        chessengine.h
)
