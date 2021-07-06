cmake_minimum_required (VERSION 2.9)
project (openexr-converter)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions ("-Wall")
add_definitions ("-Dcimg_OS=0")

include_directories(
	"3rdparty/tclap-code/include"
	"3rdparty/lodepng"
	)

add_subdirectory(src)

