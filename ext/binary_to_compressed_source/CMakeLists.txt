project(binary_to_compressed_c)
cmake_minimum_required(VERSION 3.10)
add_executable(binary_to_compressed_c ${IMGUI_SRC}/misc/fonts/binary_to_compressed_c.cpp)
install(TARGETS binary_to_compressed_c RUNTIME DESTINATION bin)
