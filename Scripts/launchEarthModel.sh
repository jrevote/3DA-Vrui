#!/bin/sh

sudo /sbin/service VRDeviceDaemon stop
sudo /sbin/service VRDeviceDaemon start
sleep 20

/usr/local/packages/Vrui-1.0-068/demos/bin/ShowEarthModel \
-quakes \
/usr/local/packages/Vrui-1.0-068/demos/EarthQuakes/1950-2010-Mag6.anss \
/usr/local/packages/Vrui-1.0-068/demos/EarthQuakes/1950-2010-Mag7.anss \
/usr/local/packages/Vrui-1.0-068/demos/EarthQuakes/1950-2010-Mag8+.anss \
/usr/local/packages/Vrui-1.0-068/demos/EarthQuakes/1950-2010-Mag5+.anss \
-rootSection dante

sudo /sbin/service VRDeviceDaemon stop
