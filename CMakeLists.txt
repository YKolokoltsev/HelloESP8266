cmake_minimum_required(VERSION 3.5)

# May be there are anu ideas?
# https://github.com/dockcross/dockcross

# Here we sugest to use C and C++ at the same time. However, if we put any compilers within a "project" statementt,
# CMake will check the default compiler that will not be used for cross-platform compliation.
# Therefore the only option here for compilers is "NONE"
# Compiler check is moved to the "enable_language" statement at the end of this file.
project(HelloESP8266 NONE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)


set(ESP_PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Your project root folder is: ${ESP_PROJECT_ROOT_DIR}")

# Prevent in source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Run cmake -H . -B <build-filder>")
endif ()

# Check if this is a host or a docker build
if(EXISTS "${ESP_PROJECT_ROOT_DIR}/esp-open-sdk")
    # When we can use local toolchain
    message(STATUS "Using local installation of esp-open-sdk (default)")
    set(ESP_OPEN_SDK_ROOT_DIR ${ESP_PROJECT_ROOT_DIR}/esp-open-sdk)
elseif(EXISTS "/tools/xtensa-lx106-elf")
    # When this script is forked from docker
    set(ESP_OPEN_SDK_ROOT_DIR /tools)
else()
    # Try to fork from docker (see ./docker/CMakeLists.txt for details)
    include("${ESP_PROJECT_ROOT_DIR}/docker/CMakeLists.txt")
    return()
endif()

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