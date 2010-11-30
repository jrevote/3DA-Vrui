/***********************************************************************
WandNavigationTool - Class encapsulating the navigation behaviour of a
classical CAVE wand.
Copyright (c) 2004-2010 Oliver Kreylos

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

#ifndef VRUI_WANDNAVIGATIONTOOL_INCLUDED
#define VRUI_WANDNAVIGATIONTOOL_INCLUDED

#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/DeviceForwarder.h>
#include <Vrui/NavigationTool.h>

namespace Vrui {

class WandNavigationTool;

class WandNavigationToolFactory:public ToolFactory
	{
	friend class WandNavigationTool;
	
	/* Elements: */
	private:
	Scalar scaleFactor; // Distance the device has to be moved along the scaling line to scale by factor of e
	
	/* Constructors and destructors: */
	public:
	WandNavigationToolFactory(ToolManager& toolManager);
	virtual ~WandNavigationToolFactory(void);
	
	/* Methods from ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class WandNavigationTool:public NavigationTool,public DeviceForwarder
	{
	friend class WandNavigationToolFactory;
	
	/* Embedded classes: */
	public:
	enum NavigationMode // Enumerated type for states the tool can be in
		{
		IDLE,PASSTHROUGH,PASSTHROUGH_MOVING,MOVING,SCALING,SCALING_PAUSED
		};
	
	/* Elements: */
	private:
	static WandNavigationToolFactory* factory; // Pointer to the factory object for this class
	InputDevice* buttonDevice; // Pointer to the input device representing the forwarded zoom button
	
	/* Transient navigation state: */
	NavigationMode navigationMode; // The tool's current navigation mode
	NavTrackerState preScale; // Transformation to be applied to the navigation transformation before scaling
	Point scalingCenter; // Center position of scaling operation
	Vector scalingDirection; // Direction of line along which is scaled
	Scalar initialScale; // Initial parameter of device position along scaling line
	NavTrackerState postScale; // Transformation to be applied to the navigation transformation after scaling
	
	/* Constructors and destructors: */
	public:
	WandNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	
	/* Methods from Tool: */
	virtual void initialize(void);
	virtual void deinitialize(void);
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	
	/* Methods from DeviceForwarder: */
	virtual std::vector<InputDevice*> getForwardedDevices(void);
	virtual InputDeviceFeatureSet getSourceFeatures(const InputDeviceFeature& forwardedFeature);
	virtual InputDevice* getSourceDevice(const InputDevice* forwardedDevice);
	virtual InputDeviceFeatureSet getForwardedFeatures(const InputDeviceFeature& sourceFeature);
	};

}

#endif
