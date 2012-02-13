#!/bin/sh

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
sudo /sbin/service VRDeviceDaemon-2.2-003 start
sleep 20

/usr/local/packages/Vrui/2.2-003/demos/bin/ShowEarthModel \
-quakes \
/usr/local/packages/Vrui/Datasets/EarthQuakes/1950-2010-Mag6.anss \
/usr/local/packages/Vrui/Datasets/EarthQuakes/1950-2010-Mag7.anss \
/usr/local/packages/Vrui/Datasets/EarthQuakes/1950-2010-Mag8+.anss \
/usr/local/packages/Vrui/Datasets/EarthQuakes/1950-2010-Mag5+.anss \
-rootSection dante

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
