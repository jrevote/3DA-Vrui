/***********************************************************************
VirtualInputDevice - Helper class to manage ungrabbed virtual input
devices.
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

#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Ray.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Sphere.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <Vrui/InputDevice.h>
#include <Vrui/Vrui.h>

#include <Vrui/VirtualInputDevice.h>

namespace Vrui {

/***********************************
Methods of class VirtualInputDevice:
***********************************/

VirtualInputDevice::VirtualInputDevice(GlyphRenderer* sGlyphRenderer,const Misc::ConfigurationFileSection&)
	:glyphRenderer(sGlyphRenderer)
	{
	buttonOffset=Vector(0,0,1.25)*Scalar(glyphRenderer->getGlyphSize());
	buttonPanelDirection=Vector(1,0,0);
	buttonSize=Scalar(0.333); // Size is relative to the input device glyph size
	buttonSpacing=Scalar(glyphRenderer->getGlyphSize())*Scalar(0.5);
	offButtonGlyph.enable(Glyph::CUBE,GLMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	onButtonGlyph.enable(Glyph::CUBE,GLMaterial(GLMaterial::Color(0.25f,1.0f,0.25f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	deviceGlyph.enable(Glyph::BOX,GLMaterial(GLMaterial::Color(0.5f,0.5f,0.5f),GLMaterial::Color(0.3f,0.3f,0.3f),5.0f));
	}

VirtualInputDevice::~VirtualInputDevice(void)
	{
	}

bool VirtualInputDevice::pick(const InputDevice* device,const Point& pos) const
	{
	typedef Geometry::Sphere<Scalar,3> Sphere;
	
	/* Test the position against the device first: */
	Sphere sphere(device->getTransformation().getOrigin(),Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575)*Math::sqrt(Scalar(3)));
	if(sphere.contains(pos))
		return true;
	
	/* Test the position against the device's buttons: */
	int numButtons=device->getNumButtons();
	sphere.setCenter(sphere.getCenter()+buttonOffset-buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(numButtons-1)));
	sphere.setRadius(sphere.getRadius()*buttonSize);
	Vector step=buttonPanelDirection*buttonSpacing;
	for(int i=0;i<numButtons;++i)
		{
		if(sphere.contains(pos))
			return true;
		sphere.setCenter(sphere.getCenter()+step);
		}
	
	return false;
	}

Scalar VirtualInputDevice::pick(const InputDevice* device,const Ray& ray) const
	{
	typedef Geometry::Sphere<Scalar,3> Sphere;
	Sphere::HitResult hitResult;
	
	/* Test the ray against the device first: */
	Sphere sphere(device->getTransformation().getOrigin(),Scalar(glyphRenderer->getGlyphSize())*Scalar(0.575)*Math::sqrt(Scalar(3)));
	if((hitResult=sphere.intersectRay(ray)).isValid())
		return hitResult.getParameter();
	
	/* Test the ray against the device's buttons: */
	int numButtons=device->getNumButtons();
	sphere.setCenter(sphere.getCenter()+buttonOffset-buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(numButtons-1)));
	sphere.setRadius(sphere.getRadius()*buttonSize);
	Vector step=buttonPanelDirection*buttonSpacing;
	for(int i=0;i<numButtons;++i)
		{
		if((hitResult=sphere.intersectRay(ray)).isValid())
			return hitResult.getParameter();
		sphere.setCenter(sphere.getCenter()+step);
		}
	
	return Math::Constants<Scalar>::max;
	}

int VirtualInputDevice::pickButton(const InputDevice* device,const Point& pos) const
	{
	Point buttonPos=device->getTransformation().getOrigin();
	buttonPos+=buttonOffset;
	buttonPos-=buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(device->getNumButtons()-1));
	Vector step=buttonPanelDirection*buttonSpacing;
	Scalar bs=Scalar(glyphRenderer->getGlyphSize())*buttonSize*Scalar(0.5);
	for(int buttonIndex=0;buttonIndex<device->getNumButtons();++buttonIndex)
		{
		bool inside=true;
		for(int i=0;i<3&&inside;++i)
			inside=buttonPos[i]-bs<=pos[i]&&pos[i]<=buttonPos[i]+bs;
		if(inside)
			return buttonIndex;
		buttonPos+=step;
		}
	
	return -1;
	}

int VirtualInputDevice::pickButton(const InputDevice* device,const Ray& ray) const
	{
	int result=-1;
	Point buttonPos=device->getTransformation().getOrigin();
	buttonPos+=buttonOffset;
	buttonPos-=buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(device->getNumButtons()-1));
	Vector buttonStep=buttonPanelDirection*buttonSpacing;
	Scalar bs=Scalar(glyphRenderer->getGlyphSize())*buttonSize*Scalar(0.5);
	Scalar lambdaMin=Math::Constants<Scalar>::max;
	for(int buttonIndex=0;buttonIndex<device->getNumButtons();++buttonIndex)
		{
		Scalar lMin=Scalar(0);
		Scalar lMax=lambdaMin;
		for(int i=0;i<3;++i)
			{
			Scalar l1,l2;
			if(ray.getDirection()[i]<Scalar(0))
				{
				l1=(buttonPos[i]+bs-ray.getOrigin()[i])/ray.getDirection()[i];
				l2=(buttonPos[i]-bs-ray.getOrigin()[i])/ray.getDirection()[i];
				}
			else if(ray.getDirection()[i]>Scalar(0))
				{
				l1=(buttonPos[i]-bs-ray.getOrigin()[i])/ray.getDirection()[i];
				l2=(buttonPos[i]+bs-ray.getOrigin()[i])/ray.getDirection()[i];
				}
			else if(buttonPos[i]-bs<=ray.getOrigin()[i]&&ray.getOrigin()[i]<buttonPos[i]+bs)
				{
				l1=Scalar(0);
				l2=lambdaMin;
				}
			else
				l1=l2=Scalar(-1);
			if(lMin<l1)
				lMin=l1;
			if(lMax>l2)
				lMax=l2;
			}
		
		if(lMin<lMax)
			{
			result=buttonIndex;
			lambdaMin=lMin;
			}
		
		/* Go to next button: */
		buttonPos+=buttonStep;
		}
	
	return result;
	}

void VirtualInputDevice::renderDevice(const InputDevice* device,const GlyphRenderer::DataItem* glyphRendererContextDataItem,GLContextData&) const
	{
	/* Get the device's current transformation: */
	OGTransform transform(device->getTransformation());
	
	/* Render glyphs for the device's buttons: */
	int numButtons=device->getNumButtons();
	OGTransform buttonTransform=OGTransform::translate(transform.getTranslation()+buttonOffset-buttonPanelDirection*(Scalar(0.5)*buttonSpacing*Scalar(numButtons-1)));
	buttonTransform*=OGTransform::scale(buttonSize);
	Vector step=buttonPanelDirection*(buttonSpacing/buttonSize);
	for(int i=0;i<numButtons;++i)
		{
		glyphRenderer->renderGlyph(device->getButtonState(i)?onButtonGlyph:offButtonGlyph,buttonTransform,glyphRendererContextDataItem);
		buttonTransform*=OGTransform::translate(step);
		}
	
	/* Render a glyph for the device: */
	glyphRenderer->renderGlyph(deviceGlyph,transform,glyphRendererContextDataItem);
	}

}
