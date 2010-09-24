/***********************************************************************
AppearanceNode - Class for appearances of shapes in VRML files.
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

#include "VRMLParser.h"

#include "AppearanceNode.h"

/*******************************
Methods of class AppearanceNode:
*******************************/

AppearanceNode::AppearanceNode(VRMLParser& parser)
	:AttributeNode(parser)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("AppearanceNode::AppearanceNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("material"))
			{
			/* Parse the material node: */
			parser.getNextToken();
			material=parser.getNextNode();
			}
		else if(parser.isToken("texture"))
			{
			/* Parse the texture node: */
			parser.getNextToken();
			texture=parser.getNextNode();
			}
		else
			Misc::throwStdErr("AppearanceNode::AppearanceNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

AppearanceNode::~AppearanceNode(void)
	{
	}

void AppearanceNode::setGLState(GLContextData& contextData) const
	{
	if(material!=0)
		material->setGLState(contextData);
	if(texture!=0)
		texture->setGLState(contextData);
	}

void AppearanceNode::resetGLState(GLContextData& contextData) const
	{
	if(texture!=0)
		texture->resetGLState(contextData);
	if(material!=0)
		material->resetGLState(contextData);
	}
