cmake_minimum_required( VERSION 3.20 )

project( space_invaders )

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )

add_subdirectory( glfw )

# include_directories("${GLFW_SOURCE_DIR}/deps")
# set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
include_directories( include )

SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )

set( space_invaders-SRC
        src/main.cpp
        src/glad.c
)

add_executable( space_invaders ${space_invaders-SRC} )

target_link_libraries( space_invaders glfw )