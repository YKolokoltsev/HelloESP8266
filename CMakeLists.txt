cmake_minimum_required(VERSION 3.5)

# May be there are anu ideas?
# https://github.com/dockcross/dockcross

#This is the cross-platform compilation. First we define compailers and their
#parameters and at the end enable them. So here it is important to define a compiler as NONE.
project(HelloESP8266 NONE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)


set(ESP_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Your project root folder is: ${ESP_PROJECT_ROOT_DIR}")

find_program(DOCKER_EXECUTABLE docker)

if(EXISTS "${ESP_PROJECT_ROOT_DIR}/esp-open-sdk")
    message(STATUS "Using local installation of the esp-open-sdk (default)")
    set(ESP_OPEN_SDK_ROOT_DIR ${ESP_PROJECT_ROOT_DIR}/esp-open-sdk)
elseif(EXISTS "/tools/xtensa-lx106-elf")
    message(STATUS "Docker build enabled")
    set(ESP_OPEN_SDK_ROOT_DIR /tools)
elseif(DOCKER_EXECUTABLE)
    message(STATUS "Local installation of the esp-open-sdk not found")
    message(STATUS "Found docker executable: ${DOCKER_EXECUTABLE}")
    message(STATUS "Rerunning CMake from within a docker")
    get_filename_component(RELATIVE_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR} NAME_WE)
    execute_process(
            COMMAND
            ${DOCKER_EXECUTABLE} run -v ${ESP_PROJECT_ROOT_DIR}:/opt/HelloESP8266 esp-open-sdk
            cmake -S /opt/HelloESP8266 -B /opt/HelloESP8266/docker-${RELATIVE_CMAKE_BINARY_DIR}
    )
    message(STATUS "Configuring done within child process")
    message(STATUS "Real build is configured within docker-${RELATIVE_CMAKE_BINARY_DIR} folder")
    return()
else()
    message(FATAL_ERROR "Failed to find suitable esp-open-sdk toolchain nor docker image")
endif()

# TODO: Try that! https://www.jetbrains.com/help/clion/docker.html

# ****************************************************************

# Toolchain, SDK and esptool
set(ESP_TOOLCHAIN_DIR ${ESP_OPEN_SDK_ROOT_DIR}/xtensa-lx106-elf)
set(ESP_SDK_DIR ${ESP_OPEN_SDK_ROOT_DIR}/sdk)
set(ESPTOOL "${ESP_OPEN_SDK_ROOT_DIR}/esptool/esptool.py")

# *****************************************************************

# Tee target platform description.
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ESP8266)

# Setup compiler, linker, ...
set(CMAKE_C_COMPILER "${ESP_TOOLCHAIN_DIR}/bin/xtensa-lx106-elf-gcc")
set(CMAKE_CXX_COMPILER "${ESP_TOOLCHAIN_DIR}/bin/xtensa-lx106-elf-g++")

# CMake uses the paths in this list as alternative roots to find filesystem
# items with find_package(), find_library() etc.
set(CMAKE_FIND_ROOT_PATH "${ESP_TOOLCHAIN_DIR} ${ESP_SDK_DIR}")

# Search programs, headers and libraries in the host environment.
# If set to ONLY, then only the roots in CMAKE_FIND_ROOT_PATH will be searched. If set to NEVER, then the
# roots in CMAKE_FIND_ROOT_PATH will be ignored and only the host system root will be used. If set to BOTH, then
# the host system paths and the paths in CMAKE_FIND_ROOT_PATH will be searched.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# ****************************************************************

set(CMAKE_EXECUTABLE_SUFFIX .out)
set(CMAKE_C_OUTPUT_EXTENSION .o)
set(CMAKE_CXX_OUTPUT_EXTENSION .o)
set(CMAKE_C_FLAGS_INIT "-nostdlib")
set(CMAKE_CXX_FLAGS_INIT "-nostdlib")

include_directories(${ESP_SDK_DIR}/include)
include_directories(${ESP_SDK_DIR}/driver_lib/include/driver)
include_directories(${ESP_PROJECT_ROOT_DIR}/include)

# ***************************************************************

set(CMAKE_C_FLAGS "-Os -Wall -Wno-unused-function -Wpointer-arith -Wundef -Werror -Wl,-EL")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline-functions -nostdlib")
#dangerous relocation: call0: call target out of range fix
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlongcalls -mtext-section-literals")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__ets__ -DICACHE_FLASH")

if (DEFINED ENV{DEBUG_ON})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_ON")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions")

# ****************************************************************

set(CMAKE_EXE_LINKER_FLAGS "-L${ESP_OPEN_SDK_ROOT_DIR}/sdk/lib -T${ESP_SDK_DIR}/ld/eagle.app.v6.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${ESP_TOOLCHAIN_DIR}/xtensa-lx106-elf/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-check-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u call_user_start -Wl,-static -O2")

# ****************************************************************
# FLAGS, CMAKE_C_LINK_FLAGS, LINK_FLAGS are empty !?!?
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -o <TARGET>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group")

# ****************************************************************

# Those libraries will be linked into every executable.
set(CMAKE_C_STANDARD_LIBRARIES "-lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -lgcc")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} -lstdc++")

# Permits to compile *.c and *.cpp projects simultaneously depending on
# the source files extention.
enable_language(C CXX)



# ****************************************************************

set(ESP_TARGET_FW1 "${CMAKE_BINARY_DIR}/0x00000.bin")
set(ESP_TARGET_FW2 "${CMAKE_BINARY_DIR}/0x10000.bin")

message(STATUS "After any compilation the binaries appear in: ${CMAKE_BINARY_DIR}")

# Create binary image prepared for flashing
function(create_image TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND
                # A new cmake instance is used as a wrapper to modify PATH variable to the
                # toolchain automaticaly. It is required by esptool.py script here.
                ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}:${ESP_TOOLCHAIN_DIR}/bin"
                ${ESPTOOL} elf2image ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.out -o ${CMAKE_BINARY_DIR}/
                BYPRODUCTS
                ${ESP_TARGET_FW1} ${ESP_TARGET_FW2}
    )
endfunction()

# ****************************************************************

add_custom_target(flash_chip
        ${ESPTOOL} --port /dev/ttyUSB0 -b 74880 write_flash -ff 80m -fm dio
        0x00000 ${ESP_TARGET_FW1}
        0x10000 ${ESP_TARGET_FW2}
        )

add_subdirectory(examples)