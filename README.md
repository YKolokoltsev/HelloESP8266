# HelloESP8266
This is a minimalistic project aimed to show the basic CMake setup steps required for programming ESP8266 chip.
Based on the [rzajac assembly](https://github.com/rzajac/esp-dev-env.git). 

In this project it is supposed that [open-esp-sdk](https://github.com/pfalcon/esp-open-sdk) is already compiled on the host machine. In this case, it is requiered to create a symlink called `open-esp-sdk` within HelloESP8266 cloned folder that points onto the open-esp-sdk. This is the only requirement for the given CMakeLists.txt script.


It is highly useful to build the toolchain within your linux-box. However, sometimes this process is not trivial due to a lots of dependencies (see the first stage of the Docker file). Therefore it is proposed the Docker script that permits to make a remote compilation within the docker container. To create a corresponding image you can use the following commands:


```
git clone https://github.com/YKolokoltsev/HelloESP8266.git && cd HelloESP8266
docker build -t esp-open-sdk docker
docker system prune --force
```

Creating of a docker image is a two stage process, and it requires at least 6 Gb of available space. The compilation takes a long time, however usually never fials.

Than you can link a local HelloESP8266 folder where you have executed all previous commands as docker container volume. In this case it is important not to execute the `/bin/bash` command within your run because this command already exist in the Dockerfile and it is a `startup-script`.

```
docker run -it -v `pwd`:/opt/HelloESP8266 esp-open-sdk
cmake -S . -B build
cd build && make
```

If under the build folder have appeared two files: `0x00000.bin` and `0x10000.bin`, than the compilation has passed successfully. These files are the firmware of the esp8266 board, and you can upload them into the flash memory.


Note: 
This documentation is under the development and is far from it's end.


Hint:
As long as `esp-open-sdk` folder is supposed to be inside of a project tree of the CLion IDE it is recommended to exlude it:
project_tree->esp-open-sdk->Mark Directory as->Excluded. This operation will speed-up considerably automatic indexing process during each build.
