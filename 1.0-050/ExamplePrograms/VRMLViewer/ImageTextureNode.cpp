/***********************************************************************
ImageTextureNode - Class for 2D textures stored as images.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <string.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>

#include "VRMLParser.h"

#include "ImageTextureNode.h"

/*******************************************
Methods of class ImageTextureNode::DataItem:
*******************************************/

ImageTextureNode::DataItem::DataItem(void)
	:textureObjectId(0)
	{
	glGenTextures(1,&textureObjectId);
	}

ImageTextureNode::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&textureObjectId);
	}

/*********************************
Methods of class ImageTextureNode:
*********************************/

ImageTextureNode::ImageTextureNode(VRMLParser& parser)
	:AttributeNode(parser),
	 imageName(0)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("url"))
			{
			parser.getNextToken();
			delete[] imageName;
			imageName=new char[parser.getLength()+1];
			strcpy(imageName,parser.getToken());
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

ImageTextureNode::~ImageTextureNode(void)
	{
	delete[] imageName;
	}

void ImageTextureNode::initContext(GLContextData& contextData) const
	{
	/* Create a new data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Upload the image texture into the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	
	Images::RGBImage image=Images::readImageFile(imageName);
	image.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB);
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

void ImageTextureNode::setGLState(GLContextData& contextData) const
	{
	/* Retrieve the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	#if 0
	/* Set up OpenGL state: */
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	
	/* Bind the texture: */
	glBindTexture(GL_TEXTURE_2D,dataItem->textureObjectId);
	#endif
	}

void ImageTextureNode::resetGLState(GLContextData& contextData) const
	{
	#if 0
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Reset OpenGL state: */
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
	glDisable(GL_TEXTURE_2D);
	#endif
	}
