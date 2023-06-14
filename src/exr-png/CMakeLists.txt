project(exr-png)
find_package(OpenEXR CONFIG)

if (NOT OpenEXR_FOUND)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")
	find_package(OpenEXR REQUIRED)
else()
	if (TARGET OpenEXR::OpenEXR)
		set(OPENEXR_LIBRARIES OpenEXR::OpenEXR Imath::Imath)
	else()
		set(OPENEXR_LIBRARIES OpenEXR::IlmImf)
	endif()
endif()

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
