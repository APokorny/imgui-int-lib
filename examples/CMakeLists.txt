add_executable(imgui-example example.cpp)
target_link_libraries(imgui-example imgui-int)

add_executable(imgui-node-example example.cpp)
target_link_libraries(imgui-node-example imgui-int imgui-node-editor)
