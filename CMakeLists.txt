cmake_minimum_required(VERSION 3.27)
project(Raytracer LANGUAGES CXX)

add_subdirectory(project)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(project/tests)
endif()
