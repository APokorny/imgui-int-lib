function(add_emscripten_environment target_n)
  set(TARGET_NAME ${target_n})
  configure_file(${TOOLCHAIN_FILE_BASE_PATH}/environment.html.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_app.html)
endfunction()
