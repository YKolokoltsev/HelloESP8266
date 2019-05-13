# HelloESP8266
This is a minimalistic project aimed to show the basic CMake setup steps required for programming ESP8266 chip.
Based on the [rzajac assembly](https://github.com/rzajac/esp-dev-env.git). 

In this project it is supposed that [open-esp-sdk](https://github.com/pfalcon/esp-open-sdk) is already compiled on the host machine. In this case, it is requiered to create a symlink called `open-esp-sdk` within HelloESP8266 cloned folder that points onto the open-esp-sdk. This is the only requirement.


NOTE: Due to complexity of manual compilation of the cross-compiler toolset, it is recommended to use it's Docker assembly provided by [larsks](https://hub.docker.com/r/larsks/esp-open-sdk/dockerfile). However, in this case the ESP8266 code should be compiled within docker container that is not the default case yet for CMake scripts present here (under the development).


HINT:
As long as here `esp-open-sdk` folder is supposed to be inside of a project tree, in CLion it should de excluded:
project_tree->esp-open-sdk->Mark Directory as->Excluded. This operation will speed-up considerably automatic indexing process during each build.
