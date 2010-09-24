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

#include <Vrui/CoordinateManager.h>

namespace Vrui {

/******************************************
Static elements of class CoordinateManager:
******************************************/

const char* CoordinateManager::unitNames[]=
	{
	"",
	"micrometer","millimeter","meter","kilometer",
	"inch","foot","yard","mile"
	};

const char* CoordinateManager::unitAbbreviations[]=
	{
	"",
	"um","mm","m","km",
	"\"","'","yd","mile"
	};

const Scalar CoordinateManager::unitInchFactors[]=
	{
	Scalar(1),
	Scalar(1.0e-3/25.4),Scalar(1.0/25.4),Scalar(1.0e3/25.4),Scalar(1.0e6/25.4),
	Scalar(1),Scalar(12),Scalar(36),Scalar(63360)
	};

/**********************************
Methods of class CoordinateManager:
**********************************/

CoordinateManager::CoordinateManager(void)
	:unit(UNKNOWN),
	 unitFactor(1)
	{
	}

void CoordinateManager::setUnit(CoordinateManager::Unit newUnit,Scalar newUnitFactor)
	{
	unit=newUnit;
	unitFactor=newUnitFactor;
	}

}
