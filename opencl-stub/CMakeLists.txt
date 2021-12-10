include_directories ("include")

file (GLOB_RECURSE SOURCES src/*.cpp)

add_library (OpenCL ${SOURCES})
