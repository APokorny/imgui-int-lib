add_executable(imgui-example example.cpp)
target_link_libraries(imgui-example Imgui::imgui)

add_executable(imgui-node-example example.cpp)
target_link_libraries(imgui-node-example Imgui::imgui imgui-node-editor)
