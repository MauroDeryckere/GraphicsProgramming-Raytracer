include(FetchContent)

FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

# Source files
set(SOURCES
    "../src/core/Matrix.cpp"
    "../src/core/Timer.cpp"
    "../src/core/Vector3.cpp"
    "../src/core/Vector4.cpp"
    "../src/rendering/Renderer.cpp"
    "../src/scene/Scene.cpp"
)

# Test source files
set(TESTS
    "UnitTests.cpp"
)

# SDL2
set(SDL_DIR "${CMAKE_SOURCE_DIR}/project/libs/SDL2-2.30.3")
add_library(SDL2_Test STATIC IMPORTED)
set_target_properties(SDL2_Test PROPERTIES
    IMPORTED_LOCATION "${SDL_DIR}/lib/SDL2.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL_DIR}/include"
)

add_executable(UnitTests ${SOURCES} ${TESTS})
target_compile_features(UnitTests PRIVATE cxx_std_20)
target_link_libraries(UnitTests gtest gtest_main SDL2_Test)

target_include_directories(UnitTests PRIVATE
    ${CMAKE_SOURCE_DIR}/project/src
    ${CMAKE_SOURCE_DIR}/project/src/core
    ${CMAKE_SOURCE_DIR}/project/src/rendering
    ${CMAKE_SOURCE_DIR}/project/src/scene
)

add_test(NAME UnitTests COMMAND UnitTests)
