#!/bin/sh

sudo /sbin/service VRDeviceDaemon-1.0-068 stop
sudo /sbin/service VRDeviceDaemon-1.0-068 start
sleep 20

/usr/local/packages/Vrui/1.0-068/demos/bin/ShowEarthModel \
/usr/local/packages/Vrui/1.0-068/demos/Data/ScatteredData/EarthquakeEvents.txt \
-color 0.0 1.0 1.0 /usr/local/packages/Vrui/1.0-068/demos/Data/ScatteredData/fltrSlab_hypo8898negdep.dat \
#-color 0.0 1.0 1.0 /usr/local/packages/Vrui/1.0-068/demos/Data/ScatteredData/Slab_hypo8898negdep.dat 
-rootSection dante

sudo /sbin/service VRDeviceDaemon-1.0-068 stop
