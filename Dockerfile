FROM ubuntu:20.04

RUN mkdir -p /etc/carsim/lua_configs

RUN apt-get update && apt-get install -y --no-install-recommends build-essential pkg-config

# Install Lua
RUN apt-get install -y --no-install-recommends lua5.2 liblua5.2-0 liblua5.2-dev

# Install Socketcan libraries
RUN apt-get install -y --no-install-recommends libsocketcan-dev

RUN mkdir -p /opt/carsimulator
COPY . /opt/carsimulator

RUN cd /opt/carsimulator && make

ENTRYPOINT cd /etc/carsim && /opt/carsimulator/dist/Debug/GNU-Linux/amos-ss17-proj4
