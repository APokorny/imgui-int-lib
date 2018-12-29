include(CMakeFindDependencyMacro)
find_dependency(freetype2 2.0)
find_dependency(GLEW 3.0)
find_dependency(SDL2 2.0) # TODO configure via build
include("${CMAKE_CURRENT_LIST_DIR}/imgui-int-Targets.cmake")
function(convert_font file symbol output)
    set(BASE ${CMAKE_CURRENT_LIST_DIR}/../bin/)
    get_filename_component(full_path ${file} ABSOLUTE)
    get_filename_component(output_path ${output} DIRECTORY)
    file(MAKE_DIRECTORY ${output_path})
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
        COMMAND "${BASE}/binary_to_compressed_c" "${full_path}" "${symbol}" > ${CMAKE_CURRENT_BINARY_DIR}/${output}
        DEPENDS binary_to_compressed_c)
endfunction()
