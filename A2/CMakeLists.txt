cmake_minimum_required(VERSION 3.9)
project(A2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)
link_directories(../lib/)
include_directories(../shared ../shared/include ../shared/gl3w ../shared/imgui)
add_executable(A2
        A2.cpp
        A2.hpp
        Main.cpp)

set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,../../lib/" )

target_link_libraries(A2 -lcs488-framework -limgui -lglfw3 -lGL -lXinerama -lXcursor -lXxf86vm -lXi -lXrandr -lX11 -lstdc++ -ldl -lpthread)
