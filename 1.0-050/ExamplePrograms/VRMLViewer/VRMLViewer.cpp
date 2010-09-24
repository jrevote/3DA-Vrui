/***********************************************************************
VRMLViewer - Main source code for the Virtual Reality VRML viewer.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdexcept>
#include <iostream>
#include <Misc/Timer.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>

#include "VRMLParser.h"
#include "VRMLNode.h"
#include "RootNode.h"

class VRMLViewer:public Vrui::Application
	{
	/* Elements: */
	private:
	VRMLNodePointer root; // Root of the VRML scene
	
	/* Constructors and destructors: */
	public:
	VRMLViewer(int& argc,char**& argv,char**& appDefaults);
	virtual ~VRMLViewer(void);
	
	/* Methods: */
	virtual void display(GLContextData& contextData) const;
	};

/***************************
Methods of class VRMLViewer:
***************************/

VRMLViewer::VRMLViewer(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults)
	{
	/* Open the input file: */
	VRMLParser parser(argv[1]);
	
	/* Read the root node: */
	Misc::Timer t1;
	root=new RootNode(parser);
	t1.elapse();
	std::cout<<"Time to parse root node: "<<t1.getTime()*1000.0<<" ms"<<std::endl;
	
	/* Calculate the root node's bounding box: */
	VRMLNode::Box bbox=root->calcBoundingBox();
	std::cout<<"Root node bounding box: ["<<bbox.getMin(0)<<", "<<bbox.getMax(0)<<"] x ["<<bbox.getMin(1)<<", "<<bbox.getMax(1)<<"] x ["<<bbox.getMin(2)<<", "<<bbox.getMax(2)<<"]"<<std::endl;
	
	/* Initialize the navigation transformation: */
	Vrui::Point center(Geometry::mid(bbox.getMin(),bbox.getMax()));
	Vrui::Scalar radius=Geometry::dist(center,Vrui::Point(bbox.getMax()));
	Vrui::setNavigationTransformation(center,radius,Vrui::Vector(0,1,0));
	}

VRMLViewer::~VRMLViewer(void)
	{
	}

void VRMLViewer::display(GLContextData& contextData) const
	{
	root->glRenderAction(contextData);
	}

/*************
Main function:
*************/

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		VRMLViewer app(argc,argv,appDefaults);
		app.run();
		return 0;
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	}
