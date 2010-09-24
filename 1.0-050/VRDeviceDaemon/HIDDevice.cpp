/***********************************************************************
HIDDevice - VR device driver class for generic input devices supported
by the Linux or MacOS X HID event interface. Reports buttons and
absolute axes.
Copyright (c) 2004-2005 Oliver Kreylos
MacOS X additions copyright (c) 2006 Braden Pellett

This file is part of the Vrui VR Device Driver Daemon (VRDeviceDaemon).

The Vrui VR Device Driver Daemon is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Vrui VR Device Driver Daemon is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui VR Device Driver Daemon; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include "HIDDevice.h"
#include "VRDeviceManager.h"

/****************
Helper functions:
****************/

struct AxisSettings
	{
	/* Elements: */
	public:
	float minValue;
	float maxValue;
	float center;
	float flat;
	};

namespace Misc {

template <>
class ValueCoder<AxisSettings>
	{
	/* Methods: */
	public:
	static std::string encode(const AxisSettings& v)
		{
		std::vector<float> values;
		values.push_back(v.minValue);
		values.push_back(v.maxValue);
		values.push_back(v.center);
		values.push_back(v.flat);
		return ValueCoder<std::vector<float> >::encode(values);
		};
	static AxisSettings decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		/* Decode string as vector of integers: */
		std::vector<float> values=ValueCoder<std::vector<float> >::decode(start,end,decodeEnd);
		if(values.size()!=4)
			throw DecodingError(std::string("Wrong number of elements in ")+std::string(start,end));
		
		/* Convert vector to result structure: */
		AxisSettings result;
		result.minValue=values[0];
		result.maxValue=values[1];
		result.center=values[2];
		result.flat=values[3];
		return result;
		};
	};

}

/*************************
System specific functions:
*************************/

#if defined (__LINUX__)
  #include "HIDDevice.Linux.cpp"
#elif defined (__DARWIN__)
  #include "HIDDevice.MacOSX.cpp"
#endif

/*************************************
Object creation/destruction functions:
*************************************/

extern "C" VRDevice* createObjectHIDDevice(VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager,Misc::ConfigurationFile& configFile)
	{
	VRDeviceManager* deviceManager=static_cast<VRDeviceManager::DeviceFactoryManager*>(factoryManager)->getDeviceManager();
	return new HIDDevice(factory,deviceManager,configFile);
	}

extern "C" void destroyObjectHIDDevice(VRDevice* device,VRFactory<VRDevice>* factory,VRFactoryManager<VRDevice>* factoryManager)
	{
	delete device;
	}
