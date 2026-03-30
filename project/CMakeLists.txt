# Source files
set(SOURCES
    src/main.cpp
    src/core/Matrix.cpp
    src/core/Timer.cpp
    src/core/Vector3.cpp
    src/core/Vector4.cpp
    src/rendering/Renderer.cpp
    src/scene/Scene.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene
)

# --- Helper: copy files to output directory ---
function(copy_files_post_build TARGET_NAME SOURCE_DIR PATTERNS)
    foreach(PATTERN ${PATTERNS})
        file(GLOB_RECURSE FILES "${SOURCE_DIR}/${PATTERN}")
        foreach(FILE ${FILES})
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE}
                $<TARGET_FILE_DIR:${TARGET_NAME}>)
        endforeach()
    endforeach()
endfunction()

function(copy_to_resources TARGET_NAME SOURCE_DIR PATTERNS)
    set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")
    file(MAKE_DIRECTORY ${OUT_DIR})
    foreach(PATTERN ${PATTERNS})
        file(GLOB_RECURSE FILES "${SOURCE_DIR}/${PATTERN}")
        foreach(FILE ${FILES})
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} ${OUT_DIR})
        endforeach()
    endforeach()
endfunction()

# Copy assets to output resources/ folder
copy_to_resources(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/assets" "*.jpg;*.png;*.obj")

# SDL2
set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/SDL2-2.30.3")
add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION "${SDL_DIR}/lib/SDL2.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL_DIR}/include"
)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2)
copy_files_post_build(${PROJECT_NAME} "${SDL_DIR}/lib" "*.dll;*.manifest")

# Visual Leak Detector (Debug only)
if(WIN32)
    set(VLD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vld")
    add_library(VLD STATIC IMPORTED)
    set_target_properties(VLD PROPERTIES
        IMPORTED_LOCATION "${VLD_DIR}/lib/vld.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${VLD_DIR}/include"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:VLD>)
    target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:ENABLE_VLD=1>)
    copy_files_post_build(${PROJECT_NAME} "${VLD_DIR}/lib" "*.dll;*.manifest")
endif()
