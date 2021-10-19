cmake_minimum_required(VERSION 3.0.0)
project(mconv64 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)

add_executable(mconv64
    src/binary_output.cpp
    src/c_output.cpp 
    src/main.cpp 
    src/mesh.cpp
)

target_link_libraries(mconv64 assimp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
