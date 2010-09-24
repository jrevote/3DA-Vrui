/***********************************************************************
TransformNode - Node class for VRML transformations.
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

#include <GL/gl.h>
#include <GL/GLTransformationWrappers.h>
#include <Misc/ThrowStdErr.h>

#include "Types.h"
#include "SFRotation.h"
#include "VRMLParser.h"

#include "TransformNode.h"

/******************************
Methods of class TransformNode:
******************************/

TransformNode::TransformNode(VRMLParser& parser)
	:GroupNode(parser),
	 transform(Transform::identity)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("TransformNode::TransformNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("translation"))
			{
			/* Parse the translation vector: */
			parser.getNextToken();
			Transform::Vector translation;
			for(int i=0;i<3;++i)
				{
				translation[i]=Transform::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			
			/* Set the transformation: */
			transform*=Transform::translate(translation);
			}
		else if(parser.isToken("rotation"))
			{
			/* Parse the rotation: */
			parser.getNextToken();
			transform*=Transform::rotate(SFRotation::parse(parser));
			}
		else if(parser.isToken("scale"))
			{
			/* Parse the scale factors: */
			parser.getNextToken();
			Transform::Vector scale;
			for(int i=0;i<3;++i)
				{
				scale[i]=Transform::Scalar(atof(parser.getToken()));
				parser.getNextToken();
				}
			
			/* Set the transformation: */
			transform*=Transform::scale(scale.mag());
			}
		else if(parser.isToken("children"))
			{
			/* Parse the node's children: */
			parseChildren(parser);
			}
		else
			Misc::throwStdErr("TransformNode::TransformNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

VRMLNode::Box TransformNode::calcBoundingBox(void) const
	{
	/* Calculate the union of the transformed bounding boxes of all children: */
	Box result=Box::empty;
	for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		{
		Box childBox=(*chIt)->calcBoundingBox();
		childBox.transform(transform);
		result.addBox(childBox);
		}
	return result;
	}

void TransformNode::glRenderAction(GLContextData& contextData) const
	{
	/* Save the current modelview matrix: */
	glPushMatrix();
	
	/* Transform the model coordinate system: */
	glMultMatrix(transform);
	
	/* Render all child nodes: */
	GroupNode::glRenderAction(contextData);
	
	/* Restore the modelview matrix: */
	glPopMatrix();
	}
