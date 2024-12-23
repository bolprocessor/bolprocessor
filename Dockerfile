FROM alpine:3.18 as bolprocessor

# RUN apk update && apk add --no-cache \
#     build-base \
#     cmake \
#     alsa-lib-dev \
#     csound \
#     wget \
#     net-tools

RUN addgroup -S audio && adduser -S daemon -G audio
RUN mkdir -p /etc/modules-load.d/
RUN mkdir -p /etc/modprobe.d/
RUN touch /etc/modules-load.d/virmidi.conf
RUN touch /etc/modprobe.d/virmidi-options.conf
ADD https://github.com/JonZudell/bolprocessor/releases/download/v0.2.11/bp3-ubuntu-latest-gcc ./bp3
RUN chmod +x bp3

CMD [ "sh" ]

# RUN DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
#     build-essential \
#     cmake \
#     build-essential \
#     libasound2-dev \
#     csound \
#     wget \
#     net-tools
    
# RUN usermod -a -G audio daemon
# RUN mkdir -p /etc/modules-load.d/
# RUN mkdir -p /etc/modprobe.d/
# RUN touch /etc/modules-load.d/virmidi.conf
# RUN touch /etc/modprobe.d/virmidi-options.conf
# ADD https://github.com/JonZudell/bolprocessor/releases/download/v0.2.11/bp3-ubuntu-latest-gcc ./bp3
# RUN chmod +x bp3

# CMD [ "sh" ]