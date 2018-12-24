
set(CMAKE_SYSTEM_NAME Emscripten)

set(EMSCRIPTEN $ENV{EMSCRIPTEN})
if("${EMSCRIPTEN}x" STREQUAL "x")
    message(ERROR " Make sure that the EMSCRIPTEN environment variable is set")
    message(ERROR " EMSCRIPTEN variable points to the emscripten SDK directory")
    message(ERROR " The SDK directory should contain a system directory with all")
    message(ERROR " prebuilt libraries.")
endif()
set(EMSCRIPTEN_SYSTEM ${EMSCRIPTEN}/system)
set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_AR "emar" CACHE FILEPATH "Emscripten ar")
set(CMAKE_RANLIB "emranlib" CACHE FILEPATH "Emscripten ranlib")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}
    "${EMSCRIPTEN_SYSTEM}"
    "${EMSCRIPTEN_PREFIX}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH})

#set(CMAKE_CXX_FLAGS_INIT "-s WASM=0")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-s WASM=1 -s USE_WEBGL2=1 -s OFFSCREENCANVAS_SUPPORT=1 -s ASSERTIONS=1 -s GL_ASSERTIONS=1")
set (CMAKE_EXECUTABLE_SUFFIX_CXX .html)
set (CMAKE_EXECUTABLE_SUFFIX_C .html)
#set(CMAKE_CXX_FLAGS_RELEASE_INIT "-DNDEBUG -O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-O3 --llvm-lto 1")
