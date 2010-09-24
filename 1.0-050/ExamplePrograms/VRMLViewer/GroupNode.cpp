/***********************************************************************
GroupNode - Base class for group nodes in VRML world files.
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

#include "GroupNode.h"

/**************************
Methods of class GroupNode:
**************************/

void GroupNode::parseChildren(VRMLParser& parser)
	{
	/* Check for the opening bracket: */
	parser.getNextToken();
	if(!parser.isToken("["))
		Misc::throwStdErr("GroupNode::GroupNode: Missing opening bracket in children attribute");
	parser.getNextToken();
	
	/* Parse child nodes until closing bracket: */
	while(!parser.isToken("]"))
		{
		/* Parse the node and add it to the group: */
		addChild(parser.getNextNode());
		
		/* Check for a comma separator and skip it: */
		if(parser.isToken(","))
			parser.getNextToken();
		}
	
	/* Skip the closing bracket: */
	parser.getNextToken();
	}

void GroupNode::addChild(VRMLNodePointer newChild)
	{
	if(newChild!=0)
		children.push_back(newChild);
	}

GroupNode::GroupNode(VRMLParser& parser)
	:VRMLNode(parser)
	{
	}

GroupNode::~GroupNode(void)
	{
	}

VRMLNode::Box GroupNode::calcBoundingBox(void) const
	{
	/* Return the union of bounding boxes of all children: */
	Box result=Box::empty;
	for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		result.addBox((*chIt)->calcBoundingBox());
	return result;
	}

void GroupNode::glRenderAction(GLContextData& contextData) const
	{
	/* Call all child nodes recursively: */
	for(NodeList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		(*chIt)->glRenderAction(contextData);
	}
