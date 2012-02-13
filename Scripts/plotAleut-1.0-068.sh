#!/bin/sh

sudo /sbin/service VRDeviceDaemon-1.0-068 stop
sudo /sbin/service VRDeviceDaemon-1.0-068 start
sleep 20

/usr/local/packages/Vrui/1.0-068/demos/bin/ShowEarthModel \
-color 255 255 0 /usr/local/packages/Vrui/1.0-068/demos/ak/Design/AleutSlab/aleutianscut_vz.dat \
-color 153 50 205 /usr/local/packages/Vrui/1.0-068/demos/ak/Design/AleutSlab/fltrSlab_hypo8898negdep.dat \
-color 151 105 79 /usr/local/packages/Vrui/1.0-068/demos/ak/Design/AleutSlab/pagewrangcont_vz.dat \
-color 33 94 33 /usr/local/packages/Vrui/1.0-068/demos/ak/Design/AleutSlab/slabcontabove50neg.xyz \
-color 143 143 189 /usr/local/packages/Vrui/1.0-068/demos/ak/Design/AleutSlab/srt4nannanNonanViz.dat \
-rootSection dante
:
sudo /sbin/service VRDeviceDaemon-1.0-068 stop
