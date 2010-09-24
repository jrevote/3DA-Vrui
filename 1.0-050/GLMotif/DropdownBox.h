/***********************************************************************
DropdownButton - Class for buttons that select one from a list of
strings using a drop-down menu.
Copyright (c) 2006 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_DROPDOWNBUTTON_INCLUDED
#define GLMOTIF_DROPDOWNBUTTON_INCLUDED

#include <string>
#include <vector>

#include <GLMotif/DecoratedButton.h>

/* Forward declarations: */
namespace GLMotif {
class Label;
class Popup;
}

namespace GLMotif {

class DropdownButton:public DecoratedButton
	{
	/* Elements: */
	protected:
	Popup* popup; // Window to pop up when button is selected
	size_t numItems; // Number of items in list
	Label** itemLabels; // Array of label widgets for the list items
	GLfloat arrowBorderSize; // Size of the cascade arrow's border
	GLfloat arrowSize; // Size of the cascade arrow
	Vector arrowOuter[7],arrowInner[7]; // Points describing the arrow
	Vector arrowNormal[7]; // Normal vectors for the arrow sides
	size_t selectedItem; // Index of the selected list item
	bool isPopped; // Flag if the popup window is displayed
	
	/* Protected methods inherited from Button: */
	void setArmed(bool newArmed);
	
	/* Protected methods inherited from DecoratedButton: */
	void drawDecoration(GLContextData& contextData) const;
	
	/* New protected methods: */
	void positionArrow(void);
	
	/* Constructors and destructors: */
	public:
	DropdownButton(const char* sName,Container* sParent,const std::vector<std::string>& items,const GLFont* sFont,bool manageChild =true);
	~DropdownButton(void);
	
	/* Methods inherited from Widget: */
	ZRange calcZRange(void) const;
	void resize(const Box& newExterior);
	void realize(GLContextData& contextData) const;
	bool findRecipient(Event& event);
	void pointerButtonDown(Event& event);
	void pointerButtonUp(Event& event);
	void pointerMotion(Event& event);
	
	/* New methods: */
	void setArrowBorderSize(GLfloat newArrowBorderSize);
	void setArrowSize(GLfloat newArrowSize);
	}

}

#endif
