#!/bin/sh

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
sudo /sbin/service VRDeviceDaemon-2.2-003 start
sleep 20

/usr/local/packages/Vrui/2.2-003/demos/bin/Jello -rootSection dante

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
