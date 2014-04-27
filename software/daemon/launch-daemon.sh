#!/bin/sh
stty -F /dev/ttyACM0 cs8 115200 ignbrk -brkint -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
cd /home/cornell/Dr-Wattson/software/daemon
/usr/bin/python2 /home/cornell/Dr-Wattson/software/daemon/network_manager.py
