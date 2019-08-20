add_executable(01_blink 01_blink.c)
target_include_directories(01_blink PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(01_blink PROPERTIES FRAMEWORK ON)
create_image(01_blink)


add_executable(02_uart_stats 02_uart_stats.c)
target_include_directories(02_uart_stats PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(02_uart_stats PROPERTIES FRAMEWORK ON)
create_image(02_uart_stats)