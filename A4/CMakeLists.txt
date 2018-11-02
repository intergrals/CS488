cmake_minimum_required(VERSION 3.9)
project(A4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)
link_directories(../lib/)
include_directories(../shared ../shared/include ../shared/gl3w ../shared/imgui)
add_executable(A4
        A4.cpp
        A4.hpp
        GeometryNode.cpp
        GeometryNode.hpp
        Image.cpp
        Image.hpp
        JointNode.cpp
        JointNode.hpp
        Light.cpp
        Light.hpp
        lua488.hpp
        Main.cpp
        Material.cpp
        Material.hpp
        Mesh.cpp
        Mesh.hpp
        PhongMaterial.cpp
        PhongMaterial.hpp
        polyroots.cpp
        polyroots.hpp
        Primitive.cpp
        Primitive.hpp
        scene_lua.cpp
        scene_lua.hpp
        SceneNode.cpp
        SceneNode.hpp)

set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,../../lib/" )

target_link_libraries(A4 -lcs488-framework -limgui -llua -llodepng -lglfw3 -lGL -lXinerama -lXcursor -lXxf86vm -lXi -lXrandr -lX11 -lstdc++ -ldl -lpthread)