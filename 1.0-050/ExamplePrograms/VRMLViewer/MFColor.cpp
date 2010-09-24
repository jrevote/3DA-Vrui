/***********************************************************************
MFColor - Class for fields containing multiple RGB color values.
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
#include <Math/Math.h>

#include "VRMLParser.h"

#include "MFColor.h"

/************************
Methods of class MFColor:
************************/

std::vector<Color> MFColor::parse(VRMLParser& parser)
	{
	std::vector<Color> result;
	
	/* Check for the opening bracket: */
	if(!parser.isToken("["))
		Misc::throwStdErr("MFColor::parse: Missing opening bracket");
	parser.getNextToken();
	
	/* Parse colors until closing bracket: */
	while(!parser.isToken("]"))
		{
		/* Parse the next color: */
		Color c(0,0,0,255);
		for(int i=0;i<3&&!parser.isToken(",")&&!parser.isToken("]");++i)
			{
			/* Parse the current token: */
			double val=atof(parser.getToken());
			if(val<0.0)
				c[i]=GLubyte(0);
			else if(val>1.0)
				c[i]=GLubyte(255);
			else
				c[i]=GLubyte(Math::floor(val*255.0+0.5));
			
			/* Go to the next token: */
			parser.getNextToken();
			}
		
		/* Store the color: */
		result.push_back(c);
		
		/* Check for a comma separator and skip it: */
		if(parser.isToken(","))
			parser.getNextToken();
		}
	
	/* Skip the closing bracket: */
	parser.getNextToken();
	
	return result;
	}