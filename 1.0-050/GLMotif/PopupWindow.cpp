/***********************************************************************
PopupWindow - Class for main windows with a draggable title bar and an
optional close button.
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

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLFont.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/TitleBar.h>

#include <GLMotif/PopupWindow.h>

namespace GLMotif {

/****************************
Methods of class PopupWindow:
****************************/

PopupWindow::PopupWindow(const char* sName,WidgetManager* sManager,const char* sTitleString,const GLFont* font)
	:Container(sName,0,false),manager(sManager),
	 titleBar(0),
	 child(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=manager->getStyleSheet();
	
	/* Create the title bar widget: */
	titleBar=new TitleBar("TitleBar",this,sTitleString,font,false);
	
	/* Set the popup window's default layout: */
	setBorderWidth(0.0f);
	setBorderType(Widget::PLAIN);
	setBorderColor(ss->borderColor);
	setBackgroundColor(ss->bgColor);
	setForegroundColor(ss->fgColor);
	childBorderWidth=ss->popupWindowChildBorderWidth;
	
	titleBar->manageChild();
	}

PopupWindow::PopupWindow(const char* sName,WidgetManager* sManager,const char* sTitleString)
	:Container(sName,0,false),manager(sManager),
	 titleBar(0),
	 child(0)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=manager->getStyleSheet();
	
	/* Create the title bar widget: */
	titleBar=new TitleBar("TitleBar",this,sTitleString,false);
	
	/* Set the popup window's default layout: */
	setBorderWidth(0.0f);
	setBorderType(Widget::PLAIN);
	setBorderColor(ss->borderColor);
	setBackgroundColor(ss->bgColor);
	setForegroundColor(ss->fgColor);
	childBorderWidth=ss->popupWindowChildBorderWidth;
	
	titleBar->manageChild();
	}

PopupWindow::~PopupWindow(void)
	{
	delete titleBar;
	delete child;
	}

Vector PopupWindow::calcNaturalSize(void) const
	{
	/* Calculate the title bar size: */
	Vector result=titleBar->calcNaturalSize();
	
	/* Calculate the child's size: */
	if(child!=0)
		{
		Vector childSize=child->calcNaturalSize();
		childSize[0]+=2.0f*childBorderWidth;
		childSize[1]+=2.0f*childBorderWidth;
		
		/* Combine the title bar and child sizes: */
		if(result[0]<childSize[0])
			result[0]=childSize[0];
		result[1]+=childSize[1];
		}
	
	return calcExteriorSize(result);
	}

ZRange PopupWindow::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Calculate the title bar's z range: */
	ZRange titleBarZRange=titleBar->calcZRange();
	if(myZRange.first>titleBarZRange.first)
		myZRange.first=titleBarZRange.first;
	if(myZRange.second<titleBarZRange.second)
		myZRange.second=titleBarZRange.second;
	
	/* Calculate the child widget's z range: */
	if(child!=0)
		{
		ZRange childZRange=child->calcZRange();
		if(myZRange.first>childZRange.first)
			myZRange.first=childZRange.first;
		if(myZRange.second<childZRange.second)
			myZRange.second=childZRange.second;
		}
	
	/* Adjust the minimum z value to accomodate the popup window's back side: */
	myZRange.first-=childBorderWidth;
	
	return myZRange;
	}

void PopupWindow::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	/* Resize the title bar: */
	Box titleBarRect=getInterior();
	GLfloat titleBarHeight=titleBar->calcNaturalSize()[1];
	titleBarRect.origin[1]+=titleBarRect.size[1]-titleBarHeight;
	titleBarRect.size[1]=titleBarHeight;
	titleBar->resize(titleBarRect);
	
	/* Resize the child: */
	if(child!=0)
		{
		Box childRect=getInterior();
		childRect.origin[0]+=childBorderWidth;
		childRect.size[0]-=2.0f*childBorderWidth;
		childRect.origin[1]+=childBorderWidth;
		childRect.size[1]-=2.0f*childBorderWidth+titleBarHeight;
		child->resize(childRect);
		}
	
	/* Resize the parent class widget again to calculate the correct z range: */
	Container::resize(newExterior);
	}

Vector PopupWindow::calcHotSpot(void) const
	{
	return titleBar->calcHotSpot();
	}

void PopupWindow::draw(GLContextData& contextData) const
	{
	/* Draw the popup window's back side: */
	Box back=getExterior().offset(Vector(0.0,0.0,getZRange().first));
	glColor(borderColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex(back.getCorner(0));
	glVertex(back.getCorner(2));
	glVertex(back.getCorner(3));
	glVertex(back.getCorner(1));
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex(back.getCorner(0));
	glVertex(back.getCorner(1));
	glVertex(getExterior().getCorner(1));
	glVertex(getExterior().getCorner(0));
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex(back.getCorner(3));
	glVertex(back.getCorner(2));
	glVertex(getExterior().getCorner(2));
	glVertex(getExterior().getCorner(3));
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex(titleBar->getExterior().getCorner(0));
	glVertex(getExterior().getCorner(2));
	glVertex(back.getCorner(2));
	glVertex(back.getCorner(0));
	glVertex(getExterior().getCorner(0));
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex(titleBar->getExterior().getCorner(1));
	glVertex(getExterior().getCorner(1));
	glVertex(back.getCorner(1));
	glVertex(back.getCorner(3));
	glVertex(getExterior().getCorner(3));
	glEnd();
	
	/* Draw the title bar: */
	titleBar->draw(contextData);
	
	/* Draw the child border: */
	Box childBox=getInterior();
	childBox.size[1]-=titleBar->getExterior().size[1];
	childBox.doInset(Vector(childBorderWidth,childBorderWidth,0.0f));
	glColor(backgroundColor);
	glBegin(GL_QUAD_STRIP);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(childBox.getCorner(0));
	glVertex(getExterior().getCorner(0));
	glVertex(childBox.getCorner(1));
	glVertex(getExterior().getCorner(1));
	glVertex(childBox.getCorner(3));
	glVertex(getExterior().getCorner(3));
	glVertex(childBox.getCorner(2));
	glVertex(getExterior().getCorner(2));
	glVertex(childBox.getCorner(0));
	glVertex(getExterior().getCorner(0));
	glEnd();
	
	/* Draw the child: */
	if(child!=0)
		child->draw(contextData);
	}

bool PopupWindow::findRecipient(Event& event)
	{
	/* Check the title bar first: */
	if(titleBar->findRecipient(event))
		return true;
	
	/* Check the child next: */
	if(child!=0&&child->findRecipient(event))
		return true;
	
	/* Check ourselves: */
	Event::WidgetPoint wp=event.calcWidgetPoint(this);
	if(isInside(wp.getPoint()))
		return event.setTargetWidget(this,wp);
	else
		return false;
	}

void PopupWindow::addChild(Widget* newChild)
	{
	if(newChild!=titleBar)
		{
		/* Delete the current child: */
		delete child;
		child=0;
		
		/* Add the new child: */
		child=newChild;
		
		/* Resize the widget: */
		resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
		}
	}

void PopupWindow::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	/* Calculate the title bar size: */
	Vector newSize=titleBar->calcNaturalSize();
	
	/* Calculate the child's size: */
	Vector childSize=newExteriorSize;
	childSize[0]+=2.0f*childBorderWidth;
	childSize[1]+=2.0f*childBorderWidth;
	
	/* Combine the title bar and child sizes: */
	if(newSize[0]<childSize[0])
		newSize[0]=childSize[0];
	newSize[1]+=childSize[1];
	
	newSize=calcExteriorSize(newSize);

	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),newSize));
	}

Widget* PopupWindow::getFirstChild(void)
	{
	/* Return the only child: */
	return child;
	}

Widget* PopupWindow::getNextChild(Widget*)
	{
	/* Since there is only one child, always return null: */
	return 0;
	}

void PopupWindow::setTitleBarColor(const Color& newTitleBarColor)
	{
	/* Set title bar color: */
	titleBar->setBorderColor(newTitleBarColor);
	titleBar->setBackgroundColor(newTitleBarColor);
	}

void PopupWindow::setTitleBarTextColor(const Color& newTitleBarTextColor)
	{
	/* Set title bar text color: */
	titleBar->setForegroundColor(newTitleBarTextColor);
	}

void PopupWindow::setTitleBorderWidth(GLfloat newTitleBorderWidth)
	{
	/* Set border width of the title bar: */
	titleBar->setBorderWidth(newTitleBorderWidth);
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void PopupWindow::setTitleString(const char* newTitleString)
	{
	/* Change the title bar's label string: */
	titleBar->setLabel(newTitleString);
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

void PopupWindow::setChildBorderWidth(GLfloat newChildBorderWidth)
	{
	/* Change the child border width: */
	childBorderWidth=newChildBorderWidth;
	
	/* Resize the widget: */
	resize(Box(Vector(0.0f,0.0f,0.0f),calcNaturalSize()));
	}

}
