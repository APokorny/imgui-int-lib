cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_minimum_required(VERSION 3.14)

project(imgui-int VERSION 1.0.0 LANGUAGES CXX)

include(ExternalProject)
include(cmake/CPM.cmake)

CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.9.0")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/ ${CMAKE_MODULE_PATH})

if(EMSCRIPTEN)
  set(VULKAN_ENABLED OFF)
  set(GL3_ENABLED OFF)
  set(GL2_ENABLED OFF)
  set(GL_INCLUDE_DIRS ${EMSCRIPTEN_SYSTEM}/include/GL)
  set(EMSCRIPRTEN_INCLUDE_DIRS ${EMSCRIPTEN_SYSTEM}/include/)
  set(GL_LIBRARIES GL)

  set(EMSCRIPTEN_ENABLED ON)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
else()
  find_package(PkgConfig)

  pkg_check_modules(GL IMPORTED_TARGET gl)
  pkg_check_modules(SDL IMPORTED_TARGET sdl2)
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
  src/calendar_widgets.cpp)
add_library(Imgui::imgui-int ALIAS imgui-int)

target_include_directories(imgui-int PRIVATE )
target_include_directories(imgui-int
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/res>
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/>
    $<INSTALL_INTERFACE:include/>
  )
target_link_libraries(imgui-int PUBLIC ImGui)
target_compile_features(imgui-int PUBLIC cxx_std_17)

if(EMSCRIPTEN) # TODO add webgpu
  include(cmake/emscripten.cmake)
  add_library(imgui-emscripten
    src/gl/renderer.cpp
    src/gl/renderer.h
    src/ui.cpp
    src/emscripten/system_integration.cpp
    src/emscripten/system_integration.h
    )
  target_link_options(imgui-emscripten PUBLIC "SHELL:-s WASM=1" "SHELL:-s USE_WEBGL2=1" "SHELL:-s GL_ASSERTIONS=1" "SHELL:-s ASSERTIONS=1")
  target_link_libraries(imgui-emscripten PUBLIC GL ImGui-gl3 imgui-int)
  install(TARGETS imgui-emscripten EXPORT imgui-intTargets)
  target_include_directories(imgui-emscripten PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/emscripten>)
  add_library(Imgui::imgui-emscripten ALIAS imgui-emscripten)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  add_library(imgui-win32-dx9
    src/win32/select_win32_renderer.cpp
    src/win32/select_win32_renderer.h
    src/win32/system_integration.cpp
    src/win32/system_integration.h
    src/dx9/renderer.cpp
    src/ui.cpp
    src/dx9/renderer.h)
  install(TARGETS imgui-win32-dx9 EXPORT imgui-intTargets)
  target_link_libraries(imgui-win32-dx9 PRIVATE d3d9 dinput wsock32 ImGui-dx9 PUBLIC imgui-int)
  add_library(Imgui::imgui-win32-dx9 ALIAS imgui-win32-dx9)
endif()
if(GL_FOUND AND SDL_FOUND)
  add_library(imgui-sdl-gl
    src/sdl/select_sdl_renderer.h
    src/sdl/select_sdl_renderer.cpp
    src/sdl/system_integration.cpp
    src/sdl/system_integration.h
    src/gl/renderer.cpp
    src/ui.cpp
    src/gl/renderer.h)
  install(TARGETS imgui-sdl-gl EXPORT imgui-intTargets)
  target_link_libraries(imgui-sdl-gl PRIVATE ImGui-gl3 ImGui-sdl dl PUBLIC imgui-int)
  target_compile_definitions(imgui-sdl-gl PRIVATE GL SDL)
  add_library(Imgui::imgui-sdl-gl ALIAS imgui-sdl-gl)
endif()
if(VULKAN_FOUND AND SDL_FOUND)
  CPMAddPackage(NAME vk-bootstrap
    GITHUB_REPOSITORY "charles-lunarg/vk-bootstrap"
    GIT_TAG master) # todo: pick a fixed revision
  add_library(imgui-sdl-vulkan
    src/sdl/select_sdl_renderer.h
    src/sdl/select_sdl_renderer.cpp
    src/sdl/system_integration.h
    src/sdl/system_integration.cpp
    src/ui.cpp
    src/vulkan/renderer.cpp
    src/vulkan/renderer.h)
  install(TARGETS imgui-sdl-vulkan EXPORT imgui-intTargets)
  target_link_libraries(imgui-sdl-vulkan PRIVATE ImGui-vulkan vk-bootstrap::vk-bootstrap ImGui-sdl PUBLIC imgui-int)
  target_compile_definitions(imgui-sdl-vulkan PRIVATE VULKAN SDL)
  install(TARGETS vk-bootstrap vk-bootstrap-compiler-warnings vk-bootstrap-vulkan-headers EXPORT vk-bootstrap-targets)
  install(EXPORT vk-bootstrap-targets DESTINATION include)
  add_library(Imgui::imgui-sdl-vulkan ALIAS imgui-sdl-vulkan)
endif()

if(TARGET imgui-sdl-vulkan)
  add_library(Imgui::imgui ALIAS imgui-sdl-vulkan)
elseif(TARGET imgui-sdl-gl)
  add_library(Imgui::imgui ALIAS imgui-sdl-gl)
elseif(TARGET imgui-emscripten)
  add_library(Imgui::imgui ALIAS imgui-emscripten)
elseif(TARGET imgui-win32-dx9)
  add_library(Imgui::imgui ALIAS imgui-win32-dx9)
endif()

option(IMGUI_INT_LIB_BUILD_EXAMPLE "Enable imgui examples" ON)
if(IMGUI_INT_LIB_BUILD_EXAMPLE)
  add_subdirectory(examples)
endif()
packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include;${PROJECT_SOURCE_DIR}/ext/imgui;${PROJECT_SOURCE_DIR}/ext/imgui-node-editor
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${PROJECT_NAME}/version.h"
  EXPORT_HEADER "${PROJECT_NAME}/export.h"
  NAMESPACE Imgui
  COMPATIBILITY AnyNewerVersion
  DISABLE_VERSION_SUFFIX YES
  ARCH_INDEPENDENT YES
  )
