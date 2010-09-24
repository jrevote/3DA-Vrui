/***********************************************************************
CascadeButton - Class for buttons that pop up secondary top-level
GLMotif UI components.
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
#include <GL/GLNormalTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Container.h>
#include <GLMotif/Popup.h>

#include <GLMotif/CascadeButton.h>

namespace GLMotif {

/******************************
Methods of class CascadeButton:
******************************/

void CascadeButton::setArmed(bool newArmed)
	{
	/* Call the base class widget's setArmed method: */
	DecoratedButton::setArmed(newArmed);
	
	/* Pop the secondary top level widget up or down: */
	if(isArmed&&!isPopped&&popup!=0)
		{
		/* Calculate the popup's transformation: */
		Vector offset=getExterior().getCorner(3);
		Vector popupHotSpot=popup->getChild()->getExterior().getCorner(2);
		offset[0]-=popupHotSpot[0];
		offset[1]-=popupHotSpot[1];
		offset[2]-=popupHotSpot[2];
		offset[2]+=getZRange().second-popup->getChild()->getZRange().first;
		getManager()->popupSecondaryWidget(this,popup,offset);
		isPopped=true;
		
		/* Calculate the extended "hit box" around the popup: */
		popupHitBox=popup->getExterior();
		ZRange popupZRange=popup->getZRange();
		popupHitBox.origin[2]=popupZRange.first;
		popupHitBox.size[2]=popupZRange.second-popupZRange.first;
		popupHitBox.doOffset(offset);
		popupHitBox.doOutset(Vector(popupExtrudeSize,popupExtrudeSize,popupExtrudeSize));
		}
	else if(!isArmed&&isPopped)
		{
		popup->getManager()->popdownWidget(popup);
		isPopped=false;
		}
	}

void CascadeButton::drawDecoration(GLContextData&) const
	{
	/* Draw the margin around the cascade arrow: */
	glColor(backgroundColor);
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(arrowOuter[0]);
	glVertex(arrowOuter[6]);
	glVertex(decorationBox.getCorner(1));
	glVertex(decorationBox.getCorner(3));
	glVertex(arrowOuter[1]);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex(decorationBox.getCorner(2));
	glVertex(arrowOuter[4]);
	glVertex(arrowOuter[3]);
	glVertex(arrowOuter[2]);
	glVertex(arrowOuter[1]);
	glVertex(decorationBox.getCorner(3));
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glVertex(decorationBox.getCorner(0));
	glVertex(decorationBox.getCorner(1));
	glVertex(arrowOuter[6]);
	glVertex(arrowOuter[5]);
	glVertex(arrowOuter[4]);
	glVertex(decorationBox.getCorner(2));
	glEnd();
	
	/* Draw the arrow sides: */
	glBegin(GL_QUADS);
	int i1=6;
	for(int i2=0;i2<7;++i2)
		{
		glNormal(arrowNormal[i1]);
		glVertex(arrowOuter[i1]);
		glVertex(arrowOuter[i2]);
		glVertex(arrowInner[i2]);
		glVertex(arrowInner[i1]);
		i1=i2;
		}
	glEnd();
	
	/* Draw the arrow face: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	for(int i=0;i<7;++i)
		glVertex(arrowInner[i]);
	glEnd();
	}

void CascadeButton::positionArrow(void)
	{
	/* Calculate the arrow corners: */
	Vector center=decorationBox.origin;
	center[0]+=0.5f*decorationBox.size[0];
	center[1]+=0.5f*decorationBox.size[1];
	
	/* Position the inner arrow points: */
	arrowInner[0]=center;
	arrowInner[0][0]+=2.0f*arrowSize;
	arrowInner[1]=center;
	arrowInner[1][1]+=2.0f*arrowSize;
	arrowInner[2]=center;
	arrowInner[2][1]+=arrowSize;
	arrowInner[3]=center;
	arrowInner[3][0]-=2.0f*arrowSize;
	arrowInner[3][1]+=arrowSize;
	arrowInner[4]=center;
	arrowInner[4][0]-=2.0f*arrowSize;
	arrowInner[4][1]-=arrowSize;
	arrowInner[5]=center;
	arrowInner[5][1]-=arrowSize;
	arrowInner[6]=center;
	arrowInner[6][1]-=2.0f*arrowSize;
	
	/* Position the outer arrow points: */
	arrowOuter[0]=arrowInner[0];
	arrowOuter[0][0]+=arrowBorderSize;
	arrowOuter[1]=arrowInner[1];
	arrowOuter[1][0]-=arrowBorderSize;
	arrowOuter[1][1]+=2.0f*arrowBorderSize;
	arrowOuter[2]=arrowInner[2];
	arrowOuter[2][0]-=arrowBorderSize;
	arrowOuter[2][1]+=arrowBorderSize;
	arrowOuter[3]=arrowInner[3];
	arrowOuter[3][0]-=arrowBorderSize;
	arrowOuter[3][1]+=arrowBorderSize;
	arrowOuter[4]=arrowInner[4];
	arrowOuter[4][0]-=arrowBorderSize;
	arrowOuter[4][1]-=arrowBorderSize;
	arrowOuter[5]=arrowInner[5];
	arrowOuter[5][0]-=arrowBorderSize;
	arrowOuter[5][1]-=arrowBorderSize;
	arrowOuter[6]=arrowInner[6];
	arrowOuter[6][0]-=arrowBorderSize;
	arrowOuter[6][1]-=2.0f*arrowBorderSize;
	
	/* Lift the inner arrow points: */
	for(int i=0;i<7;++i)
		arrowInner[i][2]+=arrowBorderSize;
	
	/* Calculate the side normal vectors: */
	arrowNormal[0]=Vector(0.577f,0.577f,0.577f);
	arrowNormal[1]=Vector(-0.707f,0.0f,0.707f);
	arrowNormal[2]=Vector(0.0f,0.707f,0.707f);
	arrowNormal[3]=Vector(-0.707f,0.0f,0.707f);
	arrowNormal[4]=Vector(0.0f,-0.707f,0.707f);
	arrowNormal[5]=Vector(-0.707f,0.0f,0.707f);
	arrowNormal[6]=Vector(0.577f,-0.577f,0.577f);
	}

CascadeButton::CascadeButton(const char* sName,Container* sParent,const char* sLabel,const GLFont* sFont,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,sFont,false),
	 popup(0),isPopped(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the arrow sizes: */
	arrowBorderSize=ss->size*0.25f;
	arrowSize=ss->size*0.25f;
	popupExtrudeSize=ss->size*4.0f;
	
	/* Set the decoration position and size: */
	setDecorationPosition(DecoratedButton::DECORATION_RIGHT);
	GLfloat width=4.0f*(arrowSize+arrowBorderSize);
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

CascadeButton::CascadeButton(const char* sName,Container* sParent,const char* sLabel,bool sManageChild)
	:DecoratedButton(sName,sParent,sLabel,false),
	 popup(0),isPopped(false)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the arrow sizes: */
	arrowBorderSize=ss->size*0.25f;
	arrowSize=ss->size*0.25f;
	popupExtrudeSize=ss->size*4.0f;
	
	/* Set the decoration position and size: */
	setDecorationPosition(DecoratedButton::DECORATION_RIGHT);
	GLfloat width=4.0f*(arrowSize+arrowBorderSize);
	setDecorationSize(Vector(width,width,0.0f));
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

CascadeButton::~CascadeButton(void)
	{
	delete popup;
	}

ZRange CascadeButton::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange zRange=DecoratedButton::calcZRange();
	
	/* Adjust for the cascade arrow: */
	if(zRange.second<getInterior().origin[2]+arrowBorderSize)
		zRange.second=getInterior().origin[2]+arrowBorderSize;
	
	return zRange;
	}
	
void CascadeButton::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	DecoratedButton::resize(newExterior);
	
	/* Position the cascade arrow: */
	positionArrow();
	}

bool CascadeButton::findRecipient(Event& event)
	{
	bool result=false;
	
	/* Find the event's point in our coordinate system: */
	Event::WidgetPoint wp=event.calcWidgetPoint(this);
	
	/* If the point is inside our bounding box, put us down as recipient: */
	if(isInside(wp.getPoint()))
		result=event.setTargetWidget(this,wp);
	
	/* If the popup is popped up, redirect the question: */
	foundChild=0;
	if(isPopped)
		{
		bool popupResult=popup->findRecipient(event);
		if(popupResult)
			{
			foundChild=event.getTargetWidget();
			event.overrideTargetWidget(this);
			result=true;
			}
		else if(popupHitBox.isInside(Vector(wp.getPoint().getComponents())))
			result|=event.setTargetWidget(this,wp);
		}
	
	return result;
	}

void CascadeButton::pointerButtonDown(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Arm the button: */
	setArmed(true);
	
	/* Find a potential event recipient in the popup: */
	if(popup->findRecipient(event))
		{
		armedChild=event.getTargetWidget();
		armedChild->pointerButtonDown(event);
		}
	else
		armedChild=0;
	}

void CascadeButton::pointerButtonUp(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Disarm the armed child: */
	if(armedChild!=0)
		{
		armedChild->pointerButtonUp(event);
		armedChild=0;
		}
	
	setArmed(false);
	}

void CascadeButton::pointerMotion(Event& event)
	{
	/* "Repair" the incoming event: */
	event.overrideTargetWidget(foundChild);
	
	/* Arm/disarm children as we go by sending fake button events: */
	if(foundChild!=armedChild)
		{
		if(armedChild!=0)
			armedChild->pointerButtonUp(event);
		armedChild=foundChild;
		if(armedChild!=0)
			armedChild->pointerButtonDown(event);
		}
	else if(armedChild!=0)
		armedChild->pointerMotion(event);
	}

void CascadeButton::setPopup(Popup* newPopup)
	{
	if(isPopped)
		{
		popup->getManager()->popdownWidget(popup);
		isPopped=false;
		}
	delete popup;
	popup=newPopup;
	}

void CascadeButton::setArrowBorderSize(GLfloat newArrowBorderSize)
	{
	arrowBorderSize=newArrowBorderSize;
	
	/* Set the decoration width: */
	GLfloat width=4.0f*(arrowSize+arrowBorderSize);
	setDecorationSize(Vector(width,width,0.0f));
	}

void CascadeButton::setArrowSize(GLfloat newArrowSize)
	{
	arrowSize=newArrowSize;
	
	/* Set the decoration width: */
	GLfloat width=4.0f*(arrowSize+arrowBorderSize);
	setDecorationSize(Vector(width,width,0.0f));
	}

void CascadeButton::setPopupExtrudeSize(GLfloat newPopupExtrudeSize)
	{
	popupExtrudeSize=newPopupExtrudeSize;
	}

}
