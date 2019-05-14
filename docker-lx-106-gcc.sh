#!/bin/bash

docker run -v /tmp:/top open_esp_sdk_stage_2 /tools/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc "$*"