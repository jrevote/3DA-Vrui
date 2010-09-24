/***********************************************************************
InputDeviceDataSaver - Class to save input device data to a file for
later playback.
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

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Vrui/Geometry.h>
#include <Vrui/InputDevice.h>
#include <Vrui/InputDeviceManager.h>

#include <Vrui/InputDeviceDataSaver.h>

namespace Vrui {

/*************************************
Methods of class InputDeviceDataSaver:
*************************************/

InputDeviceDataSaver::InputDeviceDataSaver(const char* inputDeviceDataFileName,InputDeviceManager& inputDeviceManager)
	:inputDeviceDataFile(inputDeviceDataFileName,"wb",Misc::File::LittleEndian),
	 numInputDevices(inputDeviceManager.getNumInputDevices()),
	 inputDevices(new InputDevice*[numInputDevices])
	{
	/* Save number of input devices: */
	inputDeviceDataFile.write<int>(numInputDevices);
	
	/* Save layout of all input devices in the input device manager: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Get pointer to the input device: */
		inputDevices[i]=inputDeviceManager.getInputDevice(i);
		
		/* Save input device's layout: */
		char name[40];
		strncpy(name,inputDevices[i]->getDeviceName(),40);
		name[39]='\0';
		inputDeviceDataFile.write(name,40);
		inputDeviceDataFile.write<int>(inputDevices[i]->getTrackType());
		inputDeviceDataFile.write<int>(inputDevices[i]->getNumButtons());
		inputDeviceDataFile.write<int>(inputDevices[i]->getNumValuators());
		inputDeviceDataFile.write(inputDevices[i]->getDeviceRayDirection().getComponents(),3);
		}
	}

InputDeviceDataSaver::~InputDeviceDataSaver(void)
	{
	delete[] inputDevices;
	}

void InputDeviceDataSaver::saveCurrentState(double currentTimeStamp)
	{
	/* Write current time stamp: */
	inputDeviceDataFile.write(currentTimeStamp);
	
	/* Write state of all input devices: */
	for(int i=0;i<numInputDevices;++i)
		{
		/* Write input device's tracker state: */
		if(inputDevices[i]->getTrackType()!=InputDevice::TRACK_NONE)
			{
			const TrackerState& t=inputDevices[i]->getTransformation();
			inputDeviceDataFile.write(t.getTranslation().getComponents(),3);
			inputDeviceDataFile.write(t.getRotation().getQuaternion(),4);
			}
		
		/* Write input device's button states: */
		for(int j=0;j<inputDevices[i]->getNumButtons();++j)
			{
			int buttonState=inputDevices[i]->getButtonState(j);
			inputDeviceDataFile.write(buttonState);
			}
		
		/* Write input device's valuator states: */
		for(int j=0;j<inputDevices[i]->getNumValuators();++j)
			{
			double valuatorState=inputDevices[i]->getValuator(j);
			inputDeviceDataFile.write(valuatorState);
			}
		}
	}

}
