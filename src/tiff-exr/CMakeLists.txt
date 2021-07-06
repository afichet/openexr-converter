project(tiff-exr)
find_package(OpenEXR CONFIG)

if (NOT OpenEXR_FOUND)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")
	find_package(OpenEXR REQUIRED)
else ()
	set(OPENEXR_LIBRARIES OpenEXR::OpenEXR Imath::Imath Threads::Threads)
endif ()

find_package(TIFF REQUIRED)

add_executable(tiff-exr
	main.cpp
	"../../3rdparty/lodepng/lodepng.cpp"
	)

include_directories(
	${OPENEXR_INCLUDE_PATHS}
	${TIFF_INCLUDE_DIR}
	)

target_link_libraries(tiff-exr
	${OPENEXR_LIBRARIES}
	${TIFF_LIBRARY}
	)

install (TARGETS tiff-exr
		RUNTIME DESTINATION bin)
