/***********************************************************************
Button - Base class for GLMotif UI components reacting to push events.
Copyright (c) 2001-2005 Oliver Kreylos

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

#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/Container.h>

#include <GLMotif/Button.h>

namespace GLMotif {

/***********************
Methods of class Button:
***********************/

void Button::setArmed(bool newArmed)
	{
	if(newArmed&&!isArmed)
		{
		/* Arm the button: */
		setBorderType(Widget::LOWERED);
		savedBackgroundColor=backgroundColor;
		setBackgroundColor(armedBackgroundColor);
		isArmed=true;
		}
	else if(!newArmed&&isArmed)
		{
		/* Disarm the button: */
		setBorderType(Widget::RAISED);
		setBackgroundColor(savedBackgroundColor);
		isArmed=false;
		}
	
	/* Call the arm callbacks: */
	ArmCallbackData cbData(this,isArmed);
	armCallbacks.call(&cbData);
	}

void Button::select(void)
	{
	/* Call the select callbacks: */
	SelectCallbackData cbData(this);
	selectCallbacks.call(&cbData);
	}

Button::Button(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:Label(sName,sParent,sLabel,sFont,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Button defaults to some margin: */
	setMarginWidth(ss->buttonMarginWidth);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Button::Button(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:Label(sName,sParent,sLabel,false),
	 isArmed(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Button defaults to raised border: */
	setBorderType(Widget::RAISED);
	setBorderWidth(ss->buttonBorderWidth);
	
	/* Button defaults to some margin: */
	setMarginWidth(ss->buttonMarginWidth);
	
	/* Set the armed background color: */
	armedBackgroundColor=ss->buttonArmedBackgroundColor;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

ZRange Button::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange zRange=Label::calcZRange();
	
	/* Adjust for the popping in/out when arming/disarming: */
	GLfloat baseZ=getExterior().origin[2];
	if(zRange.first>baseZ-getBorderWidth())
		zRange.first=baseZ-getBorderWidth();
	if(zRange.second<baseZ+getBorderWidth())
		zRange.second=baseZ+getBorderWidth();
	
	return zRange;
	}
	
void Button::pointerButtonDown(Event&)
	{
	/* Arm the button: */
	setArmed(true);
	}

void Button::pointerButtonUp(Event& event)
	{
	/* Select if the event is for us: */
	if(event.getTargetWidget()==this)
		select();
	
	/* Disarm the button: */
	setArmed(false);
	}

void Button::pointerMotion(Event& event)
	{
	/* Check if the new pointer position is still inside the button: */
	if(event.getTargetWidget()==this)
		{
		/* Arm the button: */
		setArmed(true);
		}
	else
		{
		/* Disarm the button: */
		setArmed(false);
		}
	}

}
