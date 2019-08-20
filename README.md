# HelloESP8266
This is a minimalistic project aimed to show basic [CMake](https://cmake.org) setup steps required for programming of the ESP8266 chip. The core approach used here was initially based on the [rzajac assembly](https://github.com/rzajac/esp-dev-env.git).  

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
ESP8266 chip has an Xtensa processor architecture. Xtensa strongly differs from the architectures present in common PC's. It means that compilation of the same C/C++ code would produce absolutely different binaries for your PC and Xtensa. Compilation for the different architecture on the host machine (computer that is used for compilation) is called cross-compilation. For many popular microcontroller architectures, for example ARM processors, required cross-compilation tools can be easily installed using system package manager. However, there is no such case yet for ESP8266 implementation of Xtensa. 

If under the build folder have appeared two files: `0x00000.bin` and `0x10000.bin`, than the compilation has passed successfully. These files are the firmware of the esp8266 board, and you can upload them into the flash memory.
One of the strongest cross-compilation toolsets known for the moment is [crosstool-NG](https://crosstool-ng.github.io/). This project sopports many architectures including Xtensa, however out of the box ESP8266 support could not be provided by crosstool-NG group because of the flexibility of Xtensa and many variations of it's implementation in different microcontrollers. As a consequence, Max Filippov created a [crosstool-NG fork](https://github.com/jcmvbkbc/crosstool-NG) that was especially oriented to the Xtensa chips including ESP8266. However, even the Max Filippov's project is too generic to start with a concrete chip. It contains just a forked crosstool-NG that is a pure GCC-based compiler and does not include any chip specific API-s. The complete toolset for ESP8266 is collected within [Paul Sokolovsky's open-esp-sdk](https://github.com/pfalcon/esp-open-sdk) project. Clean installation of the open-esp-sdk is the only requirement for HelloESP8266 and this is the point where we start the discussion.

Open-esp-sdk is distributed as a source code and shell be compiled before it's usage. It's compilation can be not a trivial task for different Linux distributions (MacOS and Windows are not considered for the moment). An example of it's installation for Debian/Ubuntu families is present on the open-esp-sdk project page, however it may depend on the operating system version. For this reason we support two different scenario of the open-esp-sdk installation. The first one and the most desirable - is the manual compilation of the open-esp-sdk on the host machine. If it was successful, __just create a symlink called `open-esp-sdk` within HelloESP8266 cloned folder that points onto the open-esp-sdk__. 

Note: 
This documentation is under the development and is far from it's end.


Hint:
As long as `esp-open-sdk` folder is supposed to be inside of a project tree of the CLion IDE it is recommended to exlude it:
*NOTE*:
As long as `esp-open-sdk` folder is supposed to be inside of a project tree, in CLion it should de excluded:
project_tree->esp-open-sdk->Mark Directory as->Excluded. This operation will speed-up considerably automatic indexing process during each build.

In the case of any problems with `open-esp-sdk` compilation we suggest the other possibility: compilation of the open-esp-sdk within the Fedora based [docker](https://hub.docker.com/r/larsks/esp-open-sdk/dockerfile) container. The second possibility is absolutely stable and controllable. However, in this case ESP8266 code should be compiled within docker container that is not the default case yet for CMake scripts present here (under development).
