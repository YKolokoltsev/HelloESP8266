######################################################################
##
## STAGE 1: Build xtensa toolchain ( ~5 Gb )
##

FROM fedora

RUN yum -y upgrade
RUN yum -y install make autoconf automake libtool gcc gcc-c++ \
	flex bison ncurses-devel expat-devel python-devel \
	git unzip wget bzip2 gperf which texinfo help2man patch \
	file xz

RUN useradd tools
RUN mkdir /tools && chown -R tools:tools /tools
USER tools
RUN git clone --recursive https://github.com/pfalcon/esp-open-sdk.git /tools
RUN cd /tools && make
######################################################################
##
## STAGE 2: This is the final image ( ~800 Mb ) - just binaries
##

FROM fedora

WORKDIR /opt/

COPY --from=0 /tools/xtensa-lx106-elf /tools/xtensa-lx106-elf
COPY --from=0 /tools/sdk /tools/sdk
COPY --from=0 /tools/esptool /tools/esptool

RUN yum -y install make autoconf automake libtool python2 pyserial which git cmake vim tree && \
git clone https://github.com/YKolokoltsev/HelloESP8266.git ./HelloESP8266 && \
ln -s /tools /opt/HelloESP8266/esp-open-sdk && \
mkdir /opt/HelloESP8266/build

ENV PATH=/tools/xtensa-lx106-elf/bin:/bin:/usr/bin
