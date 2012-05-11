#!/usr/bin/env python

import sys
import re
import math
import linecache

# Make sure that a filename is specified.
if len(sys.argv) < 3:
   print "You must supply a file name to convert and the" \
         "For example: ./preprocessSeismicData.py " \
         "earthquakes.csv earthquakes.anss\n"
   sys.exit()

# Try open the file and perform conversion.
try:
   base_file = open(sys.argv[1])
except IOError:
   print 'Cannot open: ', sys.argv[1]
else:
   try:
      output_file = open(sys.argv[2],"w")
   except IOError:
      print 'Cannon open output file for writing.'
   else:
      output_file.write("Date       "
                        "Time             "
                        "Lat       "
                        "Lon  "
                        "Depth   "
                        "Mag \n")
      output_file.write("-----------------------------------------"
                        "-------------\n")

      lat,lon,year,month,day,hour=0,0,0,0,0,0
      minute,second,mag,dep=0,0,0,0
      header=base_file.readline().replace("\n","")
      counter=0
      for header in header.split(","):
         if header=='LAT':
            lat=counter
         elif header=='LONG':
            lon=counter
         elif header=='ORI_YEAR':
            year=counter
         elif header=='ORI_MONTH':
            month=counter
         elif header=='ORI_DAY':
            day=counter
         elif header=='ORI_HOUR':
            hour=counter
         elif header=='ORI_MINUTE':
            minute=counter
         elif header=='ORI_SECOND':
            second=counter
         elif header=='MAG':
            mag=counter
         elif header=='DEPTH':
            dep=counter 
         counter+=1

      # Loop through each line and extract block values/data.
      for line in base_file.readlines():
         data = line.split(",")
         date=data[year]+"/"+data[month].zfill(2)+"/"+ \
              data[day].zfill(2)
         time=data[hour].zfill(2)+":"+data[minute].zfill(2)+ \
              ":"+str(round(float(data[second]),2)).zfill(5)
         magnitude=round(float(data[mag]),2)
         depth=round(float(data[dep]),2)
         longitude=round(float(data[lon]),4)
         latitude=round(float(data[lat]),4)
         entry=date+" "+time+" "+str(latitude).rjust(8)+" "+ \
               str(longitude).rjust(9)+" "+str(depth).rjust(6)+" "+ \
               str(magnitude).rjust(5)+"\n"
         output_file.write(entry)
      output_file.close()
   base_file.close()
