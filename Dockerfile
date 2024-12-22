FROM ghcr.io/jonzudell/bp3-ctests:v0.1.0 as bp3-ctests
FROM ghcr.io/jonzudell/php-frontend:v0.1.0 as php-frontend

FROM ubuntu:22.04 as bolprocessor

RUN DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    build-essential \
    libasound2-dev \
    csound \
    wget \
    net-tools
    
RUN usermod -a -G audio daemon
RUN mkdir -p /etc/modules-load.d/
RUN mkdir -p /etc/modprobe.d/
RUN touch /etc/modules-load.d/virmidi.conf
RUN touch /etc/modprobe.d/virmidi-options.conf
ADD https://github.com/JonZudell/bolprocessor/releases/download/v0.1.70/bp3-ubuntu-latest-gcc ./bp3-ubuntu-latest-gcc
RUN chmod +x bp3-ubuntu-latest-gcc


CMD [ "/opt/lamp/lamp" "start" ]