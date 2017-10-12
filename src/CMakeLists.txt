cmake_minimum_required (VERSION 2.6)
project(openexr-converter)
find_package(OpenEXR REQUIRED)

add_executable(openexr-converter
	main.cpp
	"../3rdparty/lodepng/lodepng.cpp"
	)

include_directories(
	"../3rdparty/tclap-code/include"
	"../3rdparty/lodepng"
	${OPENEXR_INCLUDE_PATHS}
	)

target_link_libraries(openexr-converter
	${OPENEXR_LIBRARIES}
	)

install (TARGETS openexr-converter
		RUNTIME DESTINATION bin)
