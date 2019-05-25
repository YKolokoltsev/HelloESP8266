######################################################################
##
## STAGE 1: Build xtensa toolchain ( ~5 Gb )
##

FROM fedora AS builder

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

WORKDIR /opt/HelloESP8266

COPY --from=builder /tools/xtensa-lx106-elf /tools/xtensa-lx106-elf
COPY --from=builder /tools/sdk /tools/sdk
COPY --from=builder /tools/esptool /tools/esptool

RUN yum -y install make autoconf automake libtool python2 pyserial which git cmake vim tree && \

RUN groupadd builder && \
useradd user -G builder && \
chown -R user:builder /opt/HelloESP8266

COPY ./startup-script /usr/local/bin/startup-script
RUN chmod ugo+x /usr/local/bin/startup-script

USER user

CMD startup-script

ENV PATH=/tools/xtensa-lx106-elf/bin:/bin:/usr/bin:/usr/local/bin
