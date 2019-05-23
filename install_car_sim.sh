#!/bin/bash 
###################################################################
# This file:
#
#  - Script to install and setup CAR-Simulator on Raspberry Pi 3 with PiCAN2 Duo CAN Bus Board  
#
# Usage:
#
#  ./install_pican2_duo_and_carsimulator.sh
#
# Author :  Julia Fuchs   (julia.fuchs@avl.com)
#           Emil Brunner  (emil.brunner@avl.com)
# 
#
###################################################################
#
# Zustand: Image wurde bereits instaliert und befindet sich im Raspi

clear
echo "______________________________________________________________________________"
echo ""
echo "     install CAR-Simulator on Raspberry Pi 3 with PiCAN2 Duo CAN Bus Board? "
echo ""
echo "type 'y' if you are ready"
read answer
if [ "$answer" != "y" ]
then
    exit
fi


##  Raspberry Pi vorbereiten 

# update
echo "______________________________________________________________________________"
echo ""
echo "			Update... "
echo ""
sudo apt update && sudo apt upgrade -y

# Enable SPI 
#  do_spi 0 = enable SPI
#  do_spi 1 = disable SPI
#
echo "______________________________________________________________________________"
echo ""
echo "			Enable SPI Port "
echo ""
sudo raspi-config nonint do_spi 0

# Enable MCP2515 CAN bus controllers 
echo "______________________________________________________________________________"
echo ""
echo "			Enable MCP2515 CAN bus controllers  "
echo ""
sudo bash -c 'cat << EOF >> /boot/config.txt
# Enable MCP2515 CAN bus controllers for PiCAN2 Duo CAN-Bus Board
#   CAN 1 (can0)
#   GPIO 25 Rx-IRQ
#   CAN 2 (can1)
#   GPIO 24 Rx-IRQ
#
# Syntax for Raspberry 2
#dtoverlay=mcp2515-can1-overlay,oscillator=20000000,interrupt=24
#dtoverlay=mcp2515-can0-overlay,oscillator=20000000,interrupt=25

# Syntax for Raspberry 3
dtoverlay=mcp2515-can1,oscillator=20000000,interrupt=24
dtoverlay=mcp2515-can0,oscillator=20000000,interrupt=25

dtoverlay=spi-bcm2835-overlay

EOF'


# install dependencies 
echo "______________________________________________________________________________"
echo ""
echo "			Install dependencies (git, build-essential,… ) "
echo ""
sudo apt install -y git 
sudo apt install -y build-essential raspberrypi-kernel-headers

# install Linux SocketCAN (can-utils)
echo "______________________________________________________________________________"
echo ""
echo "			Install SocketCAN (can-utils)"
echo ""
sudo apt install -y can-utils

# bring up CAN interfaces automaticly on boot
echo "______________________________________________________________________________"
echo ""
echo "			Config network/interfaces.d for can0 and can1 "
echo ""

sudo bash -c 'cat << EOF > /etc/network/interfaces.d/can0
auto can0
iface can0 inet manual
   pre-up /sbin/ip link set can0 type can bitrate 500000 triple-sampling on
   up /sbin/ifconfig can0 up
   down /sbin/ifconfig can0 down
EOF'

sudo bash -c 'cat << EOF > /etc/network/interfaces.d/can1
auto can1
iface can1 inet manual
    pre-up /sbin/ip link set can1 type can bitrate 500000 triple-sampling on
    up /sbin/ifconfig can1 up
    down /sbin/ifconfig can1 down
EOF'



# compile and install ISO-TP from hartkopp (Volkswagen AG)
# Linux Kernel Module for ISO 15765-2:2016 CAN transport protocol 
# https://github.com/hartkopp/can-isotp
# https://github.com/hartkopp/can-isotp/blob/master/README.isotp
echo "______________________________________________________________________________"
echo ""
echo "			Compile and install ISO-TP from hartkopp (Volkswagen AG)  "
echo "			Kernel Module for ISO 15765-2:2016 CAN transport protocol "
echo ""
cd ~
sudo apt install -y raspberrypi-kernel-headers
git clone https://github.com/hartkopp/can-isotp.git
cd can-isotp
sudo make
sudo make modules_install
sudo depmod
#sudo insmod ./net/can/can-isotp.ko
#sudo modprobe can-isotp



# compile and install car-simulator (AVL DiTEST GmbH)
echo "______________________________________________________________________________"
echo ""
echo "			Compile and install car-simulator (AVL DiTEST GmbH)"
echo ""

# install Lua
sudo apt install -y lua5.2 liblua5.2 liblua5.2-dev

# install unit-test for cpp
sudo apt install -y libcppunit-1.* libcppunit-dev

# clone car-simulator
cd ~
git clone -q https://github.com/AVL-DiTEST-DiagDev/car-simulator.git
cd car-simulator

# PATCH - fix path to libcrc/crcccitt.c in ecu_lua_script.cpp
cat<<EOF | patch --directory=src
*** ecu_lua_script.cpp  2019-05-02 02:22:45.081810699 +0100
--- ecu_lua_script.cpp  2019-05-02 02:25:16.741270323 +0100
***************
*** 5,11 ****
   */

  #include "ecu_lua_script.h"
! #include "crcccitt.c"
  #include "utilities.h"
  #include <iostream>
  #include <string.h>
--- 5,11 ----
   */

  #include "ecu_lua_script.h"
! #include "libcrc/crcccitt.c"
  #include "utilities.h"
  #include <iostream>
  #include <string.h>
EOF

# make car-simulator
make

# place this file into /dist/Debug/GNU-Linux/lua_config/
mkdir /dist/Debug/GNU-Linux/lua_config
cp "/home/pi/car-simulator/example luas/example.lua"  /dist/Debug/GNU-Linux/lua_config/example.lua
# for all lua functions check
# README:md in the main folder
# run the car-simulator
# /dist/Debug/GNU-Linux/amos-ss17-proj4 can0




# PiCAN2 Duo CAN Bus Board for Raspberry Pi - Functionality Test
# test SocketCAN and ISO-TP between can0 and can1

cd ~
echo "______________________________________________________________________________"
echo ""
echo "Reboot and Functionality Test"
echo ""
echo "test SocketCAN and ISO-TP between can0 and can1"

cat << EOF 
For the following tests after reboot, we assume the following:

    1. You have both termination resistors activated by closing the corresponding jumpers.
    2. You have connected both CAN Bus ports, i.e. you have connected CAN_L to CAN_L and CAN_H to CAN_H.

    can0                                 can1
    +---------+                          +---------+
    |         |                          |         |
    |   CAN_H +-----   ------   ---------> CAN_H   |
    |     |   |     \ /      \ /         |   |     |
    |   R120  |      X        X          |  R120   |
    |     |   |     / \      / \         |   |     |
    |   CAN_L +-----   ------   ---------> CAN_L   |
    |         |                          |         |
    +---------+                          +---------+
EOF

echo "______________________________________________________________________________"
# Test SocketCAN
echo "'Test SocketCAN'"
echo ""
echo "open an terminal, type:"
echo ""
echo "'candump can0'"
echo ""
echo "open an other terminal, type:"
echo ""
echo "'cansend can0 123#1122334455667788'"
echo ""
echo "you can see a message in the 'candump' terminal"
echo ""
echo "______________________________________________________________________________"
# Test ISO-TP
echo "'Test ISO-TP'"
echo ""
echo "open the receiver with: isotprecv -s 123 -d 321 -l can0"
echo ""
echo "open a new terminal and type: echo 11 22 33 44 55 66 DE AD BE EF | isotpsend -s 321 -d 123 can0"
echo ""
echo "______________________________________________________________________________"
echo ""
echo "type 'y' if you are ready to reboot"
read answer
if [ "$answer" == "y" ]
then
	sudo reboot
fi