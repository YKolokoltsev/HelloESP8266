cmake_minimum_required(VERSION 3.5)

# May be there are anu ideas?
# https://github.com/dockcross/dockcross

project(HelloESP8266 C)
set(CMAKE_C_STANDARD 99)

set(ESP_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Your project root folder is: ${ESP_PROJECT_ROOT_DIR}")

#set(ESP_OPEN_SDK_ROOT_DIR ${ESP_PROJECT_ROOT_DIR}/esp-open-sdk)
#if(NOT EXISTS ${ESP_OPEN_SDK_ROOT_DIR})
#    message(FATAL_ERROR "Searching for esp-open-sdk folder failed")
#endif()

find_program(Docker_EXECUTABLE docker)
if(NOT Docker_EXECUTABLE)
    message(FATAL_ERROR "Cannot find the docker executable!")
else()
    message(STATUS "Found docker executable: ${Docker_EXECUTABLE}")
endif()

# ****************************************************************

# Toolchain, SDK and esptool
set(ESP_TOOLCHAIN_DIR ${ESP_OPEN_SDK_ROOT_DIR}/xtensa-lx106-elf)
set(ESP_SDK_DIR ${ESP_OPEN_SDK_ROOT_DIR}/sdk)
set(ESPTOOL "${ESP_OPEN_SDK_ROOT_DIR}/esptool/esptool.py")

# *****************************************************************

# Tee target platform description.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ESP8266)

# Setup compiler, linker, ...
set(CMAKE_C_COMPILER "/home/morrigan/CLionProjects/HelloESP8266/docker-lx-106-gcc.sh")

# CMake uses the paths in this list as alternative roots to find filesystem
# items with find_package(), find_library() etc.
set(CMAKE_FIND_ROOT_PATH "${ESP_TOOLCHAIN_DIR}" "${ESP_SDK_DIR}")

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
set(CMAKE_C_FLAGS_INIT "-nostdlib")

include_directories(${ESP_SDK_DIR}/include)

# ***************************************************************

set(CMAKE_C_FLAGS "-Os -Wall -Wno-unused-function -Wpointer-arith -Wundef -Werror -Wl,-EL")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline-functions -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlongcalls -mtext-section-literals")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__ets__ -DICACHE_FLASH")

if (DEFINED ENV{DEBUG_ON})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_ON")
endif ()

# ****************************************************************

set(CMAKE_EXE_LINKER_FLAGS "-L${ESP_OPEN_SDK_ROOT_DIR}/sdk/lib -T${ESP_SDK_DIR}/ld/eagle.app.v6.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-check-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u call_user_start -Wl,-static -O2")

# ****************************************************************
# FLAGS, CMAKE_C_LINK_FLAGS, LINK_FLAGS are empty !?!?
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -o <TARGET>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group")

# ****************************************************************

# Those libraries will be linked into every executable.
set(CMAKE_C_STANDARD_LIBRARIES "-lc -lgcc -lphy -lpp -lnet80211 -llwip -lwpa -lmain")

# ****************************************************************

# Genetare targets for creating firmwares and flashing.
function(esp_gen_exec_targets TARGET_NAME)
    #set(ESP_TARGET_FW1 "${CMAKE_CURRENT_BINARY_DIR}/0x00000.bin")
    #set(ESP_TARGET_FW2 "${CMAKE_CURRENT_BINARY_DIR}/0x10000.bin")

    # Create ESP8266 binary files.
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND
            ${ESPTOOL} elf2image $<TARGET_FILE:${TARGET_NAME}> -o ${CMAKE_CURRENT_BINARY_DIR}/)
     #       BYPRODUCTS
     #       ${ESP_TARGET_FW1} ${ESP_TARGET_FW2})
endfunction()

# ****************************************************************

add_executable(${PROJECT_NAME} main.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${ESP_PROJECT_ROOT_DIR}/include)
set_target_properties( ${PROJECT_NAME} PROPERTIES FRAMEWORK ON)

esp_gen_exec_targets(${PROJECT_NAME})