cmake_minimum_required(VERSION 3.16.3)
project(raytracing)

set(CMAKE_CXX_STANDARD 17)

set(TARGET_NAME "${PROJECT_NAME}")
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(OpenMP)

include_directories(include)
add_executable(${TARGET_NAME}
        src/main.cpp
        src/BVH.cpp
        src/Camera.cpp
        src/Color.cpp
        src/geometry_utils.cpp
        src/Image.cpp
        src/parse_input.cpp
        src/Primitive.cpp
        src/Random.cpp
        src/Ray.cpp
        src/render.cpp
        src/Scene.cpp
        include/utils.hpp
)

add_subdirectory(glm)
target_link_libraries(${TARGET_NAME} glm::glm OpenMP::OpenMP_CXX)