cmake_minimum_required(VERSION 3.9)
project(A3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)
link_directories(../lib/)
include_directories(../shared ../shared/include ../shared/gl3w ../shared/imgui)
add_executable(A3
        A3.cpp
        A3.hpp
        GeometryNode.cpp
        GeometryNode.hpp
        JointNode.cpp
        JointNode.hpp
        scene_lua.cpp
        scene_lua.hpp
        SceneNode.cpp
        SceneNode.hpp
        lua488.hpp
        Material.hpp
        Main.cpp)

set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,../../lib/" )

target_link_libraries(A3 -lcs488-framework -limgui -llua -lglfw3 -lGL -lXinerama -lXcursor -lXxf86vm -lXi -lXrandr -lX11 -lstdc++ -ldl -lpthread)