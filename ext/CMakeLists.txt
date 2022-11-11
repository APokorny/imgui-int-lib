CPMAddPackage(
  NAME ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG c2694ef75e2b626ebf52493f4a1282d500d33bbb
  DOWNLOAD_ONLY YES
  PATCH_COMMAND patch -p1 -d <SOURCE_DIR>/ < ${imgui-int_SOURCE_DIR}/patches/0001-Add-layout-support-to-imgui.patch
)

macro(pack_library name inc)
  packageProject(
    NAME ${name}
    VERSION 1.84
    BINARY_DIR ${ImGui_BINARY_DIR}
    INCLUDE_DIR ${ImGui_SOURCE_DIR}/${inc}
    INCLUDE_DESTINATION include/imgui
    NAMESPACE Imgui
    COMPATIBILITY AnyNewerVersion
    DISABLE_VERSION_SUFFIX YES
    ARCH_INDEPENDENT YES
    )
endmacro()

if (ImGui_ADDED)
  add_library(ImGui
    ${ImGui_SOURCE_DIR}/imgui.h
    ${ImGui_SOURCE_DIR}/imgui.cpp
    ${ImGui_SOURCE_DIR}/imgui_internal.h
    ${ImGui_SOURCE_DIR}/imgui_tables.cpp
    ${ImGui_SOURCE_DIR}/imgui_widgets.cpp
    ${ImGui_SOURCE_DIR}/imstb_rectpack.h
    ${ImGui_SOURCE_DIR}/imstb_textedit.h
    ${ImGui_SOURCE_DIR}/imstb_truetype.h
    ${ImGui_SOURCE_DIR}/imgui_draw.cpp
    ${ImGui_SOURCE_DIR}/imgui_demo.cpp  # only for example
    ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
    ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
    )
  target_include_directories(ImGui PUBLIC
    $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:include/imgui>
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/misc/cpp>
    $<INSTALL_INTERFACE:include/imgui/misc/cpp>)

  pack_library(ImGui /)

  if(VULKAN_ENABLED)
    add_library(ImGui-vulkan
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
      )
    target_include_directories(ImGui-vulkan  PUBLIC
        $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
        $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/backends>
      )
    target_link_libraries(ImGui-vulkan PUBLIC PkgConfig::VULKAN)
    pack_library(ImGui-vulkan /backends/)
  endif()
  if(GL3_ENABLED OR EMSCRIPTEN_ENABLED)
    add_library(ImGui-gl3
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
      )
    target_include_directories(ImGui-gl3 PUBLIC
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/backends>
      )
    if(GL3_ENABLED)
      target_link_libraries(ImGui-gl3 PUBLIC PkgConfig::GL)
    endif()
    pack_library(ImGui-gl3 /backends/)
  endif()
  if(GL2_ENABLED)
    add_library(ImGui-gl2
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_opengl2.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
      )
    target_include_directories(ImGui-gl2 PUBLIC
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/backends>
      )
    target_link_libraries(ImGui-gl2 PUBLIC PkgConfig::GL)
    pack_library(ImGui-gl2 /backends/)
  endif()
  if(SDL_FOUND)
    add_library(ImGui-sdl
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
      )
    target_link_libraries(ImGui-sdl PUBLIC PkgConfig::SDL)
    target_include_directories(ImGui-sdl PUBLIC
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/backends>
      )
    pack_library(ImGui-sdl /backends/)
  endif()
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_library(ImGui-dx9 
      # make this runtime selectable?
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx10.h
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx10.cpp
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx11.h
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx12.h
      #${ImGui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_dx9.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_win32.h
      ${ImGui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
      )
    target_include_directories(ImGui-dx9
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/>
      $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}/backends>
      )
    pack_library(ImGui-dx9 /backends/)
  endif()

  ExternalProject_Add(binary_to_compressed_c
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/binary_to_compressed_source/"
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}" "-DIMGUI_SRC=${ImGui_SOURCE_DIR}"
      )

  function(convert_font file symbol output)
      set(BASE ${imgui-int_BINARY_DIR}/ext/bin/)
      get_filename_component(full_path ${file} ABSOLUTE)
      get_filename_component(output_path ${output} DIRECTORY)
      file(MAKE_DIRECTORY ${output_path})
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
          COMMAND "${BASE}/binary_to_compressed_c" "${full_path}" "${symbol}" > ${CMAKE_CURRENT_BINARY_DIR}/${output}
          DEPENDS binary_to_compressed_c)
  endfunction()

  if(FT_FOUND)
      add_library(ImGui-freetype
        ${ImGui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
          )
      target_include_directories(ImGui-freetype PUBLIC ${FT_INCLUDEDIR})
      target_compile_options(ImGui-freetype PUBLIC ${FT_CFLAGS})
      target_link_libraries(ImGui-freetype ImGui ${FT_LIBRARY})
      target_include_directories(ImGui-freetype PRIVATE
        $<BUILD_INTERFACE:${ImGui_SOURCE_DIR}>)

    pack_library(ImGui-freetype "")
  endif()
endif()
CPMAddPackage(
  NAME imgui-node-editor
  GIT_REPOSITORY https://github.com/thedmd/imgui-node-editor.git
  GIT_TAG 99ec923a39062f556ec7676fb9ba6d302d109f76
  DOWNLOAD_ONLY YES
)
if(imgui-node-editor_ADDED)
  add_library(stb-image  INTERFACE)
  target_include_directories(stb-image INTERFACE $<BUILD_INTERFACE:${imgui-node-editor_SOURCE_DIR}/external/stb_image/>)
  packageProject(
    NAME stb-image
    VERSION 2.19
    BINARY_DIR ${imgui-node-editor_BINARY_DIR}
    INCLUDE_DIR ${imgui-node-editor_SOURCE_DIR}/external/stb_image/
    INCLUDE_DESTINATION include/stb_image
    NAMESPACE Imgui
    COMPATIBILITY AnyNewerVersion
    DISABLE_VERSION_SUFFIX YES
    ARCH_INDEPENDENT YES
    )

  add_library(imgui-node-editor
      ${imgui-node-editor_SOURCE_DIR}/crude_json.h
      ${imgui-node-editor_SOURCE_DIR}/crude_json.cpp
      ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.h
      ${imgui-node-editor_SOURCE_DIR}/imgui_bezier_math.inl
      ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.h
      ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.h
      ${imgui-node-editor_SOURCE_DIR}/imgui_extra_math.inl
      ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.h
      ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.h
      ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_internal.inl
      ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.cpp
      ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.cpp
      ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_api.cpp
      )
  target_link_libraries(imgui-node-editor PUBLIC ImGui stb-image)
  target_include_directories(imgui-node-editor PUBLIC
          $<BUILD_INTERFACE:${imgui-node-editor_SOURCE_DIR}/>)
  packageProject(
    NAME imgui-node-editor
    VERSION 1.84
    BINARY_DIR ${imgui-node-editor_BINARY_DIR}
    INCLUDE_DIR ${imgui-node-editor_SOURCE_DIR}
    INCLUDE_DESTINATION include/imgui-node-editor
    VERSION_HEADER "imgui-node-editor/version.h"
    EXPORT_HEADER "imgui-node-editor/export.h"
    NAMESPACE Imgui
    COMPATIBILITY AnyNewerVersion
    DISABLE_VERSION_SUFFIX YES
    ARCH_INDEPENDENT YES
    )
endif()
