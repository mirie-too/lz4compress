cmake_minimum_required(VERSION 2.8)

project(lz4compress)

# all *.h && *.cc files in src
file(GLOB_RECURSE sources
    "src/*.h"
    "src/*.cc"
)

# lz4
set(LZ4_DIR ./lz4-r121/)
include_directories(${LZ4_DIR})
set(lz4_sources ${LZ4_DIR}lz4.c ${LZ4_DIR}lz4hc.c ${LZ4_DIR}lz4.h ${LZ4_DIR}lz4hc.h)
source_group(lz4 FILES ${lz4_sources})

# tclap
include_directories(tclap-1.2.1/include)

set(OUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUT_DIR})
if (MSVC)
	# avoid generating executables in bin/Debug & bin/Release
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUT_DIR})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})
endif()

add_executable(${PROJECT_NAME} ${sources} ${lz4_sources})

if (MSVC)
	# set additional compiler & linker flags
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/WX")	# treat warnings as errors
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)								# needed for protobuf
endif()

# configure executable names
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_DEBUG "${PROJECT_NAME}_d")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME_RELEASE "${PROJECT_NAME}")
