/***********************************************************************
MeasurementTool - Tool to measure positions, distances and angles in
physical or navigational coordinates.
Copyright (c) 2006-2008 Oliver Kreylos

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

#ifndef VRUI_MEASUREMENTTOOL_INCLUDED
#define VRUI_MEASUREMENTTOOL_INCLUDED

#include <string>
#include <GLMotif/RadioBox.h>
#include <Vrui/Geometry.h>
#include <Vrui/Tools/UtilityTool.h>

/* Forward declarations: */
namespace Misc {
class File;
}
namespace GLMotif {
class PopupWindow;
class TextField;
}

namespace Vrui {

class MeasurementTool;

class MeasurementToolFactory:public ToolFactory
	{
	friend class MeasurementTool;
	
	/* Embedded classes: */
	public:
	enum MeasurementMode
		{
		POSITION,DISTANCE,ANGLE
		};
	
	enum CoordinateMode
		{
		PHYSICAL,NAVIGATIONAL
		};
	
	/* Elements: */
	private:
	MeasurementMode defaultMeasurementMode; // Measurement mode for new tools
	CoordinateMode defaultCoordinateMode; // Coordinate mode for new tools
	Scalar markerSize; // Size of measurement markers in physical units
	bool saveMeasurements; // Flag whether measurement tools save their measurements to a text file
	std::string measurementFileName; // Name of text file where measurements will be saved
	Misc::File* measurementFile; // Pointer to measurement file; will be created on demand and shared by all measurement tools during an application's lifetime
	
	/* Constructors and destructors: */
	public:
	MeasurementToolFactory(ToolManager& toolManager);
	virtual ~MeasurementToolFactory(void);
	
	/* Methods: */
	virtual Tool* createTool(const ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Tool* tool) const;
	};

class MeasurementTool:public UtilityTool
	{
	friend class MeasurementToolFactory;
	
	/* Elements: */
	static MeasurementToolFactory* factory; // Pointer to the factory object for this class
	GLMotif::PopupWindow* measurementDialogPopup; // Pointer to the measurement dialog window
	GLMotif::TextField* pos[3][3]; // The coordinate text fields for positions 1, 2, and 3
	GLMotif::TextField* dist[2]; // The distance text fields for distances 1 and 2
	GLMotif::TextField* angle; // The angle text field
	MeasurementToolFactory::MeasurementMode measurementMode; // Measurement mode
	int numMeasurementPoints; // Number of points to measure in the current mode
	MeasurementToolFactory::CoordinateMode coordinateMode; // Coordinate mode
	
	/* Transient state: */
	int numPoints; // The number of selected measurement points
	Point points[3]; // The two measurement points
	bool dragging; // Flag if the tool is currently dragging one of its measurement points
	
	/* Private methods: */
	void resetTool(void); // Resets the tool's measurement state
	void changeMeasurementModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void changeCoordinateModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	
	/* Constructors and destructors: */
	public:
	MeasurementTool(const ToolFactory* factory,const ToolInputAssignment& inputAssignment);
	virtual ~MeasurementTool(void);
	
	/* Methods: */
	virtual const ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int deviceIndex,int buttonIndex,InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

}

#endif
