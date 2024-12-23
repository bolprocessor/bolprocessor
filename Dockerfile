FROM ubuntu:18.04 as bolprocessor

RUN DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libasound2-dev \
    csound \
    wget \
    net-tools
    
RUN usermod -a -G audio daemon
RUN mkdir -p /etc/modules-load.d/
RUN mkdir -p /etc/modprobe.d/
RUN touch /etc/modules-load.d/virmidi.conf
RUN touch /etc/modprobe.d/virmidi-options.conf
ADD https://github.com/JonZudell/bolprocessor/releases/download/v0.2.15/bp3-ubuntu-latest-gcc ./bp3
RUN chmod +x bp3

CMD [ "sh" ]