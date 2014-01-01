cmake_minimum_required(VERSION 2.6)

project(SSE_Stuff)

# Use modified FindSDL2 and FindGLEW that will work with my windows setup
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${SSE_Stuff_SOURCE_DIR}/cmake")

# Bump up warning levels appropriately for each compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c99 -msse")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	add_definitions(-DDEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
else()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(include)
include_directories(${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})

add_subdirectory(src)

