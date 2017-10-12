cmake_minimum_required (VERSION 2.6)
project (openexr-converter)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_definitions ("-Wall")
add_definitions ("-Dcimg_OS=0")

add_subdirectory(src)

