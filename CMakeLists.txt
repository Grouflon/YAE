cmake_minimum_required(VERSION 3.2)

# Architecture
set(CMAKE_BUILD_ARCH "x64" CACHE STRING "")
message(${CMAKE_BUILD_ARCH})

include(ExternalProject)

set(EXTERNAL_DIR "${CMAKE_BINARY_DIR}/extern")
set(DEPENDENCY_EXTRACT_DIR "${CMAKE_BINARY_DIR}/extract")

# SDL library
set(SDL2_VERSION 2.0.12)
set(SDL2_LIB_DIR ${EXTERNAL_DIR}/sdl2/lib)
set(SDL2_INCLUDE_DIR ${EXTERNAL_DIR}/sdl2/include)
ExternalProject_Add(
  sdl2
  PREFIX ${DEPENDENCY_EXTRACT_DIR}
  URL http://libsdl.org/release/SDL2-devel-${SDL2_VERSION}-VC.zip
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCY_EXTRACT_DIR}/src/sdl2/lib     ${SDL2_LIB_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCY_EXTRACT_DIR}/src/sdl2/include	${SDL2_INCLUDE_DIR}
)

project(Yae VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/$<CONFIG>/)

find_package(OpenGL REQUIRED)
#find_package(MIRROR REQUIRED HINTS "./extern/mirror")

file(GLOB YAE_SRC "./src/**.h" "./src/**.cpp")
#file(GLOB IMGUI_SRC "./extern/imgui/*.h" "./extern/imgui/*.cpp")

add_compile_definitions(
	_CRT_SECURE_NO_WARNINGS
)

set(SOURCES
	${YAE_SRC}
)

include_directories(
	./src/
	${SDL2_INCLUDE_DIR}
)

add_executable(yae "./src/main.cpp" ${SOURCES})
add_dependencies(yae sdl2)

target_link_libraries(yae
	#d3d9.lib
	${SDL2_LIB_DIR}/${CMAKE_BUILD_ARCH}/SDL2.lib
	${SDL2_LIB_DIR}/${CMAKE_BUILD_ARCH}/SDL2Main.lib
)

add_custom_command(TARGET yae POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${SDL2_LIB_DIR}/${CMAKE_BUILD_ARCH}/SDL2.dll
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

add_custom_target(yae_run
	DEPENDS yae
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMENT "Running executable..."
  COMMAND yae
)
