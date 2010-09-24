/***********************************************************************
VisletManager - Class to manage vislet classes.
Copyright (c) 2006-2007 Oliver Kreylos

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

#ifndef VRUI_VISLETMANAGER_INCLUDED
#define VRUI_VISLETMANAGER_INCLUDED

#include <vector>
#include <Misc/ConfigurationFile.h>
#include <Plugins/FactoryManager.h>
#include <Vrui/Vislet.h>

/* Forward declarations: */
class GLContextData;

namespace Vrui {

class VisletManager:public Plugins::FactoryManager<VisletFactory>
	{
	/* Embedded classes: */
	private:
	typedef std::vector<Vislet*> VisletList; // Data type for list of loaded vislets
	
	/* Elements: */
	Misc::ConfigurationFileSection configFileSection; // The vislet manager's configuration file section - valid throughout the manager's entire lifetime
	VisletList vislets; // List of all loaded vislets
	
	/* Constructors and destructors: */
	public:
	VisletManager(const Misc::ConfigurationFileSection& sConfigFileSection); // Initializes vislet manager by reading given configuration file section
	~VisletManager(void); // Destroys vislet manager and all loaded vislets
	
	/* Methods: */
	Misc::ConfigurationFileSection getVisletClassSection(const char* visletClassName) const; // Returns the configuration file section a vislet class should use for its initialization
	Vislet* createVislet(VisletFactory* factory,int numVisletArguments,const char* const visletArguments[]); // Loads a vislet of the given class and initializes it with the given list of parameters
	void frame(void); // Calls the frame function of all loaded vislets
	void display(GLContextData& contextData) const; // Renders all loaded vislets
	};

}

#endif
