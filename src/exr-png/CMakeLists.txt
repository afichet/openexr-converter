project(exr-png)
find_package(OpenEXR REQUIRED)

add_executable(exr-png
	main.cpp
	"../../3rdparty/lodepng/lodepng.cpp"
	)

include_directories(
	${OPENEXR_INCLUDE_PATHS}
	)

target_link_libraries(exr-png
	${OPENEXR_LIBRARIES}
	)

install (TARGETS exr-png
		RUNTIME DESTINATION bin)
