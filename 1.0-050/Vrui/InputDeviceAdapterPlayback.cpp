/***********************************************************************
InputDeviceAdapterPlayback - Class to read input device states from a
pre-recorded file for playback and/or movie generation.
Copyright (c) 2004-2005 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Misc/Endianness.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Constants.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceManager.h>

#include <Vrui/InputDeviceAdapterPlayback.h>

namespace Vrui {

/*******************************************
Methods of class InputDeviceAdapterPlayback:
*******************************************/

InputDeviceAdapterPlayback::InputDeviceAdapterPlayback(InputDeviceManager* sInputDeviceManager,const Misc::ConfigurationFileSection& configFileSection)
	:InputDeviceAdapter(sInputDeviceManager),
	 inputDeviceDataFile(configFileSection.retrieveString("./inputDeviceDataFileName").c_str(),"rb",Misc::File::LittleEndian),
	 timeStamp(0.0),done(false)
	{
	/* Read file header: */
	numInputDevices=inputDeviceDataFile.read<int>();
	inputDevices=new InputDevice*[numInputDevices];
	
	/* Initialize devices: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Read device's layout from file: */
		DeviceFileHeader header;
		inputDeviceDataFile.read(header.name,sizeof(header.name));
		inputDeviceDataFile.read(header.trackType);
		inputDeviceDataFile.read(header.numButtons);
		inputDeviceDataFile.read(header.numValuators);
		inputDeviceDataFile.read(header.deviceRayDirection.getComponents(),3);
		
		/* Create new input device: */
		InputDevice* newDevice=inputDeviceManager->createInputDevice(header.name,header.trackType,header.numButtons,header.numValuators,true);
		newDevice->setDeviceRayDirection(header.deviceRayDirection);
		
		/* Store the input device: */
		inputDevices[i]=newDevice;
		}
	
	/* Read time stamp of next data frame: */
	nextTimeStamp=inputDeviceDataFile.read<double>();
	if(inputDeviceDataFile.eof())
		{
		done=true;
		nextTimeStamp=Math::Constants<double>::max;
		}
	}

InputDeviceAdapterPlayback::~InputDeviceAdapterPlayback(void)
	{
	}

void InputDeviceAdapterPlayback::updateInputDevices(void)
	{
	/* Do nothing if at end of file: */
	if(done)
		return;
	
	/* Update time stamp: */
	timeStamp=nextTimeStamp;
	
	/* Update all input devices: */
	for(int device=0;device<numInputDevices;++device)
		{
		/* Update tracker state: */
		if(inputDevices[device]->getTrackType()!=InputDevice::TRACK_NONE)
			{
			TrackerState::Vector translation;
			inputDeviceDataFile.read(translation.getComponents(),3);
			Scalar quat[4];
			inputDeviceDataFile.read(quat,4);
			inputDevices[device]->setTransformation(TrackerState(translation,TrackerState::Rotation::fromQuaternion(quat)));
			}
		
		/* Update button states: */
		for(int i=0;i<inputDevices[device]->getNumButtons();++i)
			{
			int buttonState=inputDeviceDataFile.read<int>();
			inputDevices[device]->setButtonState(i,buttonState);
			}
		
		/* Update valuator states: */
		for(int i=0;i<inputDevices[device]->getNumValuators();++i)
			{
			double valuatorState=inputDeviceDataFile.read<double>();
			inputDevices[device]->setValuator(i,valuatorState);
			}
		}
	
	/* Read time stamp of next data frame: */
	try
		{
		nextTimeStamp=inputDeviceDataFile.read<double>();
		}
	catch(Misc::File::ReadError)
		{
		done=true;
		nextTimeStamp=Math::Constants<double>::max;
		}
	}

}
