/***********************************************************************
Lightsource - Class to describe light sources in virtual environments.
Since light sources in OpenGL are a limited resource, Vrui contains an
abstraction that allows users to create light sources as needed, and
maps created light sources to OpenGL light sources as needed.
Copyright (c) 2005 Oliver Kreylos

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

#include <Vrui/Lightsource.h>

namespace Vrui {

/****************************
Methods of class Lightsource:
****************************/

Lightsource::Lightsource(void)
	:enabled(true)
	{
	}

Lightsource::Lightsource(const GLLight& sLight)
	:enabled(true),light(sLight)
	{
	}

void Lightsource::enable(void)
	{
	enabled=true;
	}

void Lightsource::disable(void)
	{
	enabled=false;
	}

}
