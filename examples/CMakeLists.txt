set(backends )
macro(add_backends)
    set(possible_backends imgui-sdl-vulkan imgui-sdl-gl imgui-emscripten imgui-win32-dx9)
    foreach(item ${possible_backends})
        if(TARGET ${item})
            list(APPEND backends ${item})
        endif()
    endforeach()
endmacro()
add_backends()

foreach(backend ${backends})
    message(status "BACKEND: ${backend}")
    add_executable(imgui-example-${backend} example.cpp)
    target_link_libraries(imgui-example-${backend} ${backend})

    add_executable(imgui-node-example-${backend} node-example.cpp)
    target_link_libraries(imgui-node-example-${backend} ${backend} imgui-node-editor)

    add_executable(imgui-canvas-example-${backend} canvas-example.cpp)
    target_link_libraries(imgui-canvas-example-${backend} ${backend} imgui-node-editor)

    add_executable(imgui-interaction-example-${backend} interaction-example.cpp)
    target_link_libraries(imgui-interaction-example-${backend} ${backend} imgui-node-editor)

    add_executable(imgui-blueprints-example-${backend} blueprints-example.cpp)
    target_link_libraries(imgui-blueprints-example-${backend} ${backend} imgui-node-editor stb-image)
endforeach()
