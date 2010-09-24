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

#ifndef IMAGETEXTURENODE_INCLUDED
#define IMAGETEXTURENODE_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

#include "AttributeNode.h"

class ImageTextureNode:public AttributeNode,public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint textureObjectId; // ID of the texture object holding the image texture
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	char* imageName; // The name of the image texture
	
	/* Constructors and destructors: */
	public:
	ImageTextureNode(VRMLParser& parser);
	virtual ~ImageTextureNode(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void setGLState(GLContextData& contextData) const;
	virtual void resetGLState(GLContextData& contextData) const;
	};

#endif
