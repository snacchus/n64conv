cmake_minimum_required(VERSION 3.0.0)
project(tconv64 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)

add_executable(tconv64 
    src/main.cpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
