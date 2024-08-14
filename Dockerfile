FROM ghcr.io/jonzudell/bp3-ctests:v0.1.0 as bp3-ctests
FROM ghcr.io/jonzudell/php-frontend:v0.1.0 as php-frontend

FROM ubuntu:22.04 as bolprocessor

RUN DEBIAN_FRONTEND=noninteractive apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    build-essential \
    libasound2-dev \
    csound \
    wget

RUN wget https://sourceforge.net/projects/xampp/files/XAMPP%20Linux/8.2.12/xampp-linux-x64-8.2.12-0-installer.run/download -O xampp-installer.run
RUN chmod +x xampp-installer.run
RUN ./xampp-installer.run

WORKDIR /opt/lampp/htdocs/bolprocessor/
# copy from the bp3-ctests image
COPY --from=bp3-ctests /bp3-ctests /opt/lampp/htdocs/bolprocessor/bp3-ctests
COPY --from=php-frontend /php-frontend/php /opt/lampp/htdocs/bolprocessor/
COPY --from=php-frontend /php-frontend/csound_resources /opt/lampp/htdocs/bolprocessor/php-frontend/csound_resources

# copy the bolprocessor source code
COPY ./source/ /opt/lampp/htdocs/bolprocessor/
COPY ./Makefile /opt/lampp/htdocs/bolprocessor/
COPY ./change_permissions.sh /opt/lampp/htdocs/bolprocessor/
COPY ./BP2_help.txt /opt/lampp/htdocs/bolprocessor/
