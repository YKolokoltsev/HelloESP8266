#For *.cpp source files the xtensa-lx106-elf-g++ compiler and linker are used.

add_executable(cpp_01_blink 01_blink.cpp)
create_image(cpp_01_blink)