include(CMakeFindDependencyMacro)
find_dependency(freetype2 2.0)
find_dependency(GLEW 3.0)
find_dependency(SDL2 2.0) # TODO configure via build
include("${CMAKE_CURRENT_LIST_DIR}/imgui-int-Targets.cmake")
