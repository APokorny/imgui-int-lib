if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(imgui-int VERSION 1.0.0 LANGUAGES CXX)
cmake_minimum_required(VERSION 3.10)

include(ExternalProject)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/ ${CMAKE_MODULE_PATH})


if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(VULKAN_ENABLED OFF)
    set(GL3_ENABLED OFF)
    set(GL2_ENABLED OFF)
    set(GL_INCLUDE_DIRS ${EMSCRIPTEN_SYSTEM}/include/GL)
    set(EMSCRIPRTEN_INCLUDE_DIRS ${EMSCRIPTEN_SYSTEM}/include/)
    set(GL_LIBRARIES GL)

    set(EMSCRIPTEN_ENABLED ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
    find_package(PkgConfig)

    pkg_check_modules(GL IMPORTED_TARGET gl)
    pkg_check_modules(SDL IMPORTED_TARGET sdl2)
    pkg_check_modules(GLEW IMPORTED_TARGET glew)
    pkg_check_modules(FT IMPORTED_TARGET freetype2)
    pkg_check_modules(VULKAN IMPORTED_TARGET vulkan)

    set(VULKAN_ENABLED ${VULKAN_FOUND})
    set(GL3_ENABLED ${GL_FOUND})
    set(GL2_ENABLED ${GL_FOUND})
    set(EMSCRIPTEN_ENABLED OFF)
endif()

add_subdirectory(ext)

add_library(imgui-int
    include/imgui/renderer.h
    include/imgui/style.h
    include/imgui/default_style.h
    include/imgui/context.h
    include/imgui/ui.h
    include/imgui/system_integration.h
    src/context.cpp
    src/default_style.cpp
    src/calendar_widgets.cpp
    src/ui.cpp)
add_library(Imgui::imgui-int ALIAS imgui-int)

if(EMSCRIPTEN)
    target_sources(imgui-int
        PRIVATE
          src/gl/renderer.cpp
          src/gl/renderer.h
          src/emscripten/system_integration.cpp
          src/emscripten/system_integration.h
          )
    target_link_libraries(imgui-int PUBLIC GL) # TODO GL finder does not support emscripten yet..
    target_include_directories(imgui-int PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/emscripten>)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_sources(imgui-int
        PRIVATE
           src/win32/select_win32_renderer.cpp
           src/win32/select_win32_renderer.h
           src/win32/system_integration.cpp
           src/win32/system_integration.h
           src/dx9/renderer.cpp
           src/dx9/renderer.h)
    target_link_libraries(imgui-int PRIVATE d3d9 dinput wsock32)
elseif(GL_FOUND AND GLEW_FOUND)
    target_sources(imgui-int
        PRIVATE
           src/gl/renderer.cpp
           src/gl/renderer.h)
    target_link_libraries(imgui-int PRIVATE PkgConfig::GL PkgConfig::GLEW)
    target_compile_options(imgui-int PRIVATE -DGL)
endif()

if(SDL_FOUND AND NOT WIN32)
    target_sources(imgui-int
        PRIVATE
           src/sdl/select_sdl_renderer.h
           src/sdl/select_sdl_renderer.cpp
           src/sdl/system_integration.cpp
           src/sdl/system_integration.h)
    target_link_libraries(imgui-int PRIVATE PkgConfig::SDL)
    target_compile_options(imgui-int PRIVATE -DSDL)
endif()

target_link_libraries(imgui-int PRIVATE ImGui ImGui-cpp)
target_include_directories(imgui-int
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/res/
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/imgui/misc/cpp>
        $<INSTALL_INTERFACE:include/imgui/misc/cpp>
        $<INSTALL_INTERFACE:include/>
    )

option(BUILD_EXAMPLE ON)
add_subdirectory(examples)

install(TARGETS imgui-int EXPORT imgui-int-Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )

install(EXPORT imgui-int-Targets
    FILE imgui-int-Targets.cmake
    NAMESPACE Imgui::
    DESTINATION lib/cmake/
    )

include(CMakePackageConfigHelpers)
write_basic_package_version_file("imgui-int-ConfigVersion.cmake"
    VERSION "${imgui-int_VERSION}"
    COMPATIBILITY SameMajorVersion
    )
install(FILES
    "imgui-int-Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/imgui-int-ConfigVersion.cmake"
    DESTINATION lib/cmake/
    )
