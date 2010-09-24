/***********************************************************************
CoordinateManager - Class to manage the (navigation) coordinate system
of a Vrui application to support system-wide navigation manipulation
interfaces.
Copyright (c) 2007 Oliver Kreylos

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

#ifndef VRUI_COORDINATEMANAGER_INCLUDED
#define VRUI_COORDINATEMANAGER_INCLUDED

#include <Vrui/Geometry.h>

namespace Vrui {

class CoordinateManager
	{
	/* Embedded classes: */
	public:
	enum Unit // Enumerated type for coordinate units
		{
		UNKNOWN=0,
		
		/* Metric units: */
		MICROMETER,MILLIMETER,METER,KILOMETER,
		
		/* Imperial units: */
		INCH,FOOT,YARD,MILE,
		
		NUM_COORDINATEUNITS
		};
	
	/* Elements: */
	private:
	static const char* unitNames[NUM_COORDINATEUNITS]; // Full names of coordinate units for display
	static const char* unitAbbreviations[NUM_COORDINATEUNITS]; // Abbreviated names of coordinate units for display
	static const Scalar unitInchFactors[NUM_COORDINATEUNITS]; // Conversion factors from coordinate units to inches
	
	/* Current coordinate system settings: */
	Unit unit; // Type of coordinate unit used by application
	Scalar unitFactor; // Multiplication factor for coordinate unit used by application (i.e., one application unit = unitFactor coordinateUnit)
	
	/* Constructors and destructors: */
	public:
	CoordinateManager(void); // Creates coordinate manager with default settings (unknown unit with factor 1)
	
	/* Methods: */
	void setUnit(Unit newUnit,Scalar newUnitFactor); // Sets the application's coordinate unit and scale factor
	const char* getUnitName(void) const // Returns the full name of the current application coordinate unit
		{
		return unitNames[unit];
		}
	const char* getUnitAbbreviation(void) const // Returns the abbreviated name of the current application coordinate unit
		{
		return unitAbbreviations[unit];
		}
	Scalar getUnitInchFactor(void) const // Returns conversion factor from current coordinate unit to inches
		{
		return unitInchFactors[unit];
		}
	};

}

#endif
