/***********************************************************************
InlineNode - Class for VRML nodes referring to an external VRML file.
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

#include <stdlib.h>
#include <Misc/ThrowStdErr.h>

#include "VRMLParser.h"

#include "InlineNode.h"

/***************************
Methods of class InlineNode:
***************************/

InlineNode::InlineNode(VRMLParser& parser)
	:GroupNode(parser),
	 boundingBox(VRMLNode::Box::empty)
	{
	bool haveExplicitBoundingBox=false;
	VRMLNode::Box::Point bboxCenter=VRMLNode::Box::Point::origin;
	VRMLNode::Box::Size bboxSize(0,0,0);
	
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("InlineNode::InlineNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("bboxCenter"))
			{
			/* Parse the bounding box center: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				bboxCenter[i]=VRMLNode::Box::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			haveExplicitBoundingBox=true;
			}
		else if(parser.isToken("bboxSize"))
			{
			/* Parse the bounding box size: */
			parser.getNextToken();
			for(int i=0;i<3;++i)
				{
				bboxSize[i]=VRMLNode::Box::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			haveExplicitBoundingBox=true;
			}
		else if(parser.isToken("url"))
			{
			/* Parse the external VRML file name: */
			parser.getNextToken();
			
			/* Load the external VRML file: */
			VRMLParser externalParser(parser.getToken());
			
			/* Read nodes from the external VRML file until end-of-file: */
			while(!externalParser.eof())
				{
				/* Read the next node and add it to the group: */
				addChild(externalParser.getNextNode());
				}
			
			parser.getNextToken();
			}
		else
			Misc::throwStdErr("InlineNode::InlineNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	
	/* Compute the bounding box: */
	if(haveExplicitBoundingBox)
		boundingBox=VRMLNode::Box(bboxCenter,bboxSize);
	else
		{
		/* Compute the bounding box as the union of all child nodes' bounding boxes: */
		boundingBox=GroupNode::calcBoundingBox();
		}
	}
