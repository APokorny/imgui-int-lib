add_library(ImGui
    imgui/imgui.h
    imgui/imgui_internal.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp  # only for example
    )

target_include_directories(ImGui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:include/>)

add_library(ImGui-cpp
    imgui/misc/cpp/imgui_stdlib.h
    imgui/misc/cpp/imgui_stdlib.cpp
    )
target_include_directories(ImGui-cpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp>
    $<INSTALL_INTERFACE:include/imgui/misc/cpp>)
target_include_directories(ImGui-cpp PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/>)
target_link_libraries(ImGui-cpp PUBLIC ImGui)

ExternalProject_Add(binary_to_compressed_c
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/binary_to_compressed_source/"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}"
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
        imgui/misc/freetype/imgui_freetype.cpp
        )
    target_include_directories(ImGui-freetype PUBLIC ${FT_INCLUDEDIR})
    target_compile_options(ImGui-freetype PUBLIC ${FT_CFLAGS})
    target_link_libraries(ImGui-freetype ImGui ${FT_LIBRARY})
    target_include_directories(ImGui-freetype PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/>)
endif()

install(FILES imgui/imgui.h
    DESTINATION include/imgui)

install(TARGETS ImGui EXPORT imgui-int-Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )

install(TARGETS ImGui-cpp EXPORT imgui-int-Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
