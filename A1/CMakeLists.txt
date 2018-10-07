cmake_minimum_required(VERSION 3.9)
project(A1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)
link_directories(../lib/)
include_directories(../shared ../shared/include ../shared/gl3w ../shared/imgui)
add_executable(A1
        A1.cpp
        A1.hpp
        Main.cpp
        maze.cpp
        maze.hpp)

set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,../../lib/" )

target_link_libraries(A1 -lcs488-framework -limgui -lglfw3 -lGL -lXinerama -lXcursor -lXxf86vm -lXi -lXrandr -lX11 -lstdc++ -ldl -lpthread)

