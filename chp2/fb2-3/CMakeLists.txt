execute_process(COMMAND
flex -o ${CMAKE_CURRENT_SOURCE_DIR}/2.06_include_file_and_start_state.c
${CMAKE_CURRENT_SOURCE_DIR}/2.06_include_file_and_start_state.l)

add_executable(2.06_include_file_and_start_state
2.06_include_file_and_start_state.c)
