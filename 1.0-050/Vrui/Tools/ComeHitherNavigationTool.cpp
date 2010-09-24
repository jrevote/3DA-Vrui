/***********************************************************************
ComeHitherNavigationTool - Class to navigate by smoothly moving the
position of a 3D input device to the display center point.
Copyright (c) 2008 Oliver Kreylos

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

#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>

#include <Vrui/Tools/ComeHitherNavigationTool.h>

namespace Vrui {

/*****************************************
Methods of class ComeHitherNavigationToolFactory:
*****************************************/

ComeHitherNavigationToolFactory::ComeHitherNavigationToolFactory(ToolManager& toolManager)
	:ToolFactory("ComeHitherNavigationTool",toolManager),
	 moveTime(2.0)
	{
	/* Initialize tool layout: */
	layout.setNumDevices(1);
	layout.setNumButtons(0,1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* navigationToolFactory=toolManager.loadClass("NavigationTool");
	navigationToolFactory->addChildClass(this);
	addParentClass(navigationToolFactory);
	
	/* Load class settings: */
	Misc::ConfigurationFileSection cfs=toolManager.getToolClassSection(getClassName());
	moveTime=cfs.retrieveValue<double>("./moveTime",moveTime);
	
	/* Set tool class' factory pointer: */
	ComeHitherNavigationTool::factory=this;
	}


ComeHitherNavigationToolFactory::~ComeHitherNavigationToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	ComeHitherNavigationTool::factory=0;
	}

Tool* ComeHitherNavigationToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new ComeHitherNavigationTool(this,inputAssignment);
	}

void ComeHitherNavigationToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveComeHitherNavigationToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("NavigationTool");
	}

extern "C" ToolFactory* createComeHitherNavigationToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	ComeHitherNavigationToolFactory* comeHitherNavigationToolFactory=new ComeHitherNavigationToolFactory(*toolManager);
	
	/* Return factory object: */
	return comeHitherNavigationToolFactory;
	}

extern "C" void destroyComeHitherNavigationToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/*************************************************
Static elements of class ComeHitherNavigationTool:
*************************************************/

ComeHitherNavigationToolFactory* ComeHitherNavigationTool::factory=0;

/*****************************************
Methods of class ComeHitherNavigationTool:
*****************************************/

ComeHitherNavigationTool::ComeHitherNavigationTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment)
	:NavigationTool(factory,inputAssignment)
	{
	}

const ToolFactory* ComeHitherNavigationTool::getFactory(void) const
	{
	return factory;
	}

void ComeHitherNavigationTool::buttonCallback(int,int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState&&!isActive()) // Button has just been pressed and tool is not already active
		{
		/* Try activating this tool: */
		if(activate())
			{
			/* Get the start navigation transformation: */
			startNav=getNavigationTransformation();
			startTime=getApplicationTime();
			
			/* Get the target transformation: */
			NavTransform device=input.getDevice(0)->getTransformation();
			device.leftMultiply(getInverseNavigationTransformation());
			Point center=device.getOrigin();
			Vector forward=device.getDirection(1);
			Vector up=device.getDirection(2);
			
			/* Compute the navigation transformation for the target transformation: */
			targetNav=NavTransform::identity;
			targetNav*=NavTransform::translateFromOriginTo(getDisplayCenter());
			targetNav*=NavTransform::rotate(Rotation::fromBaseVectors(Geometry::cross(getForwardDirection(),getUpDirection()),getForwardDirection()));
			targetNav*=NavTransform::scale(startNav.getScaling());
			targetNav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(Geometry::cross(forward,up),forward)));
			targetNav*=NavTransform::translateToOriginFrom(center);
			
			/* Compute the linear and angular velocities for the movement: */
			NavTransform delta=startNav;
			delta.doInvert();
			delta.leftMultiply(targetNav);
			linearVelocity=delta.getTranslation()/Scalar(factory->moveTime);
			angularVelocity=delta.getRotation().getScaledAxis()/Scalar(factory->moveTime);
			
			#if 0
			/* Set the final navigation transformation: */
			setNavigationTransformation(targetNav);
			
			/* Deactivate this tool: */
			deactivate();
			#endif
			}
		}
	}

void ComeHitherNavigationTool::frame(void)
	{
	/* Act depending on this tool's current state: */
	if(isActive())
		{
		/* Get the current application time: */
		double deltaTime=getApplicationTime()-startTime;
		
		if(deltaTime>=factory->moveTime)
			{
			/* Set the final navigation transformation: */
			setNavigationTransformation(targetNav);
			
			/* Deactivate this tool: */
			deactivate();
			}
		else
			{
			/* Compute and set the intermediate navigation transformation: */
			NavTransform delta=NavTransform(linearVelocity*deltaTime,Rotation::rotateScaledAxis(angularVelocity*deltaTime),Scalar(1));
			delta*=startNav;
			setNavigationTransformation(delta);
			}
		}
	}

}
