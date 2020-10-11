set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR "x86_64")
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32/
    /usr/share/mingw-w64/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(COMPILER_PREFIX x86_64-w64-mingw32)

set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-c++-posix)
set(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc-posix)
set(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)