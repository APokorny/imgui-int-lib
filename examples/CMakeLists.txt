add_executable(imgui-example example.cpp)
target_link_libraries(imgui-example Imgui::imgui)

add_executable(imgui-node-example node-example.cpp)
target_link_libraries(imgui-node-example Imgui::imgui imgui-node-editor)

add_executable(imgui-canvas-example canvas-example.cpp)
target_link_libraries(imgui-canvas-example Imgui::imgui imgui-node-editor)
