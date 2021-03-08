cmake_minimum_required(VERSION 3.2)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# Architecture
set(CMAKE_BUILD_ARCH "x64" CACHE STRING "")
message(${CMAKE_BUILD_ARCH})

#[[include(ExternalProject)

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
)]]

project(Yae VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "/NODEFAULTLIB:MSVCRT" )

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/$<CONFIG>/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/$<CONFIG>/)

#find_package(OpenGL REQUIRED)
#find_package(MIRROR REQUIRED HINTS "./extern/mirror")

file(GLOB_RECURSE YAE_SRC "./src/**.h" "./src/**.cpp")
file(GLOB IMGUI_SRC "./extern/imgui/*.h" "./extern/imgui/*.cpp")
#file(GLOB IMGUI_SRC "./extern/imgui/*.h" "./extern/imgui/*.cpp")

add_compile_definitions(
	_CRT_SECURE_NO_WARNINGS
	_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
	GLFW_INCLUDE_VULKAN
	GLM_FORCE_RADIANS
	GLM_FORCE_DEPTH_ZERO_TO_ONE
)

set(SOURCES
	${YAE_SRC}
	${IMGUI_SRC}
)

# Group the previous files using the folder tree 
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${SOURCES})

include_directories(
	./src/
	./extern/GLFW/include/
	./extern/glm/
	./extern/stb/
	./extern/tinyobjloader/
	./extern/imgui/
	$ENV{VK_SDK_PATH}/include/
)

link_directories(
	./extern/GLFW/lib-vc2019/
	$ENV{VK_SDK_PATH}/Lib/
)

add_executable(yae "./src/main.cpp" ${SOURCES})
#add_dependencies(yae sdl2)

target_link_libraries(yae
	glfw3
	vulkan-1
	#User32.lib
	#Gdi32.lib
	#Shell32.lib
)

#[[add_custom_command(TARGET yae POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${SDL2_LIB_DIR}/${CMAKE_BUILD_ARCH}/SDL2.dll
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)]]

#[[add_custom_target(yae_run
	DEPENDS yae
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMENT "Running executable..."
  COMMAND yae
)]]
