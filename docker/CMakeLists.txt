# TODO: Try that! https://www.jetbrains.com/help/clion/docker.html

message(STATUS "Failed to find local installation of the esp-open-sdk, trying docker")

# check if the host machine has docker installed
find_program(DOCKER_EXEC docker)
if(DOCKER_EXEC)
    message(STATUS "Docker found at: ${DOCKER_EXEC}")
else()
    message(FATAL_ERROR "Failed to find docker")
endif()

# check if there is an esp-open-sdk docker image already compiled (OS dependent code, for teh meoment linux only)
message("\nListing images...")
execute_process(
        COMMAND
            ${DOCKER_EXEC} image ls
        OUTPUT_VARIABLE
            FOUND_IMAGES
)
message("\n${FOUND_IMAGES}")

if(FOUND_IMAGES MATCHES "esp-open-sdk")
    message(STATUS "Docker image found (esp-open-sdk)")
    message(STATUS "CMake configuration forked to docker")
else()
    message(FATAL_ERROR "Expected docker image not found,  install esp-open-sdk image first!")
endif()

# Get current binary dir name (it depends from user preferences or the IDE selected)
get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME_WE)

execute_process(
        COMMAND
        ${DOCKER_EXECUTABLE} run -v ${ESP_PROJECT_ROOT_DIR}:/opt/HelloESP8266 esp-open-sdk
        cmake -S /opt/HelloESP8266 -B /opt/HelloESP8266/docker-${BINARY_DIR_NAME}
)

message(STATUS "Docker configuring done")
message(STATUS "Main build folder is now: docker-${BINARY_DIR_NAME}")

# TODO!!!
# For now we are at the host machine. All makefiles are configured in the docker-${BINARY_DIR_NAME} folder
# and shall be executed within docker. Now we have to map that compilation to the host!