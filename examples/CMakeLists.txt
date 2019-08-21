add_executable(01_blink 01_blink.c)
target_include_directories(01_blink PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(01_blink PROPERTIES FRAMEWORK ON)
create_image(01_blink)


add_executable(02_stats 02_stats.c)
target_include_directories(02_stats PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(02_stats PROPERTIES FRAMEWORK ON)
create_image(02_stats)

add_executable(03_station 03_station.c)
target_include_directories(03_station PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(03_station PROPERTIES FRAMEWORK ON)
create_image(03_station)

add_executable(04_udp_flood 04_udp_flood.c)
target_include_directories(04_udp_flood PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties(04_udp_flood PROPERTIES FRAMEWORK ON)
create_image(04_udp_flood)