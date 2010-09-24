/***********************************************************************
VRMLParser - Class to parse certain parts of the geometry definitions of
VRML 2.0 / VRML 97 files.
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

#include <ctype.h>
#include <string.h>
#include <iostream>

#include "VRMLParser.h"
#include "TransformNode.h"
#include "ShapeNode.h"
#include "AppearanceNode.h"
#include "MaterialNode.h"
#include "ImageTextureNode.h"
#include "IndexedFaceSetNode.h"
#include "TextureCoordinateNode.h"
#include "ColorNode.h"
#include "NormalNode.h"
#include "CoordinateNode.h"
#include "LODNode.h"
#include "InlineNode.h"

namespace {

/****************
Helper functions:
****************/

inline bool isSeparator(char c)
	{
	return c==','||c=='{'||c=='['||c=='}'||c==']';
	}

}

/***************************
Methods of class VRMLParser:
***************************/

void VRMLParser::addTokenChar(char newChar)
	{
	/* Increase the token buffer size if the buffer is full: */
	if(tokenLength==tokenBufferSize)
		{
		/* Increase token buffer size exponentially: */
		tokenBufferSize=(tokenBufferSize*3+1)/2;
		
		/* Copy the old token into the new buffer: */
		char* newToken=new char[tokenBufferSize];
		memcpy(newToken,token,tokenLength);
		delete[] token;
		token=newToken;
		}
	
	/* Add the new character to the token: */
	token[tokenLength]=newChar;
	++tokenLength;
	}

VRMLParser::VRMLParser(const char* vrmlFileName)
	:vrmlFile(vrmlFileName,"rt",Misc::File::DontCare),
	 nextChar(' '),
	 tokenBufferSize(10),
	 token(new char[tokenBufferSize]),
	 tokenLength(0),
	 nodeDictionary(101)
	{
	/* Read the VRML file header: */
	char header[80];
	do
		{
		vrmlFile.gets(header,sizeof(header));
		}
	while(header[0]!='#');
	if(strncmp(header+1,"VRML V2.0",9)!=0)
		Misc::throwStdErr("VRMLParser::VRMLParser: %s is not a valid VRML 2.0 file",vrmlFileName);
	
	/* Skip characters until the next end-of-line: */
	while(header[strlen(header)-1]!='\n')
		vrmlFile.gets(header,sizeof(header));
	
	/* Read the next token: */
	getNextToken();
	}

VRMLParser::~VRMLParser(void)
	{
	/* Delete the token buffer: */
	delete[] token;
	}

void VRMLParser::getNextToken(void)
	{
	/* Skip whitespace from the current file position: */
	while(isspace(nextChar))
		{
		nextChar=vrmlFile.getc();
		
		/* Check for comments: */
		if(nextChar=='#')
			{
			/* Skip comment character: */
			nextChar=vrmlFile.getc();
			
			/* Skip characters until the next newline: */
			while(nextChar!='\n'&&nextChar!='\r')
				nextChar=vrmlFile.getc();
			}
		}
	
	/* Process this token: */
	if(nextChar==EOF) // Check for end-of-file
		{
		/* Clear the token: */
		tokenLength=0;
		}
	else if(nextChar=='"') // Check for quoted tokens
		{
		/* Skip the opening quote: */
		nextChar=vrmlFile.getc();
		
		/* Process characters until the closing quote: */
		tokenLength=0;
		while(nextChar!='"')
			{
			/* Process escape characters: */
			if(nextChar=='\\')
				{
				/* Get the escaped character: */
				nextChar=vrmlFile.getc();
				}
			
			/* Add the character to the curren token: */
			addTokenChar(char(nextChar));
			
			/* Read the next character: */
			nextChar=vrmlFile.getc();
			}
		
		/* Skip the closing quote: */
		nextChar=vrmlFile.getc();
		}
	else if(isSeparator(nextChar)) // Check for single-character tokens
		{
		/* Store the character as the current token: */
		tokenLength=0;
		addTokenChar(char(nextChar));
		
		/* Read the next character: */
		nextChar=vrmlFile.getc();
		}
	else
		{
		/* Process characters until the next separator: */
		tokenLength=0;
		while(!isSeparator(nextChar)&&!isspace(nextChar))
			{
			/* Add the character to the curren token: */
			addTokenChar(char(nextChar));
			
			/* Read the next character: */
			nextChar=vrmlFile.getc();
			}
		}
	
	/* Add a terminating NUL to the token: */
	addTokenChar('\0');
	--tokenLength; // The NUL doesn't count for token length
	}

VRMLNodePointer VRMLParser::getNextNode(void)
	{
	/* Check if the next token is a definition: */
	NodeName nodeName;
	if(isToken("DEF"))
		{
		/* Skip the DEF token: */
		getNextToken();
		
		/* Store the node name: */
		nodeName=token;
		
		/* Skip the name token: */
		getNextToken();
		}
	
	VRMLNodePointer result;
	if(isToken("USE"))
		{
		/* Skip the USE token: */
		getNextToken();
		
		/* Retrieve the node of the given name: */
		result=nodeDictionary.getEntry(token).getDest();
		
		/* Skip the name token: */
		getNextToken();
		
		return result;
		}
	else if(isToken("PROTO"))
		{
		/* Skip the PROTO token: */
		getNextToken();
		
		/* Skip the prototype name: */
		std::cout<<"Skipping prototype definition of name "<<getToken()<<std::endl;
		getNextToken();
		
		/* Skip the opening bracket: */
		if(!isToken("["))
			Misc::throwStdErr("VRMLParser::getNextNode: Missing opening bracket in prototype interface declaration");
		getNextToken();
		
		/* Skip the interface declaration: */
		while(!isToken("]"))
			getNextToken();
		
		/* Skip the closing bracket: */
		getNextToken();
		
		/* Skip the opening brace: */
		if(!isToken("{"))
			Misc::throwStdErr("VRMLParser::getNextNode: Missing opening brace in prototype body definition");
		getNextToken();
		
		/* Skip tokens until the matching closing brace: */
		unsigned int braceLevel=1;
		while(braceLevel>0)
			{
			if(isToken("{"))
				++braceLevel;
			else if(isToken("}"))
				--braceLevel;
			getNextToken();
			}
		
		result=0;
		}
	else if(isToken("Transform"))
		{
		/* Parse a transformation node: */
		getNextToken();
		result=new TransformNode(*this);
		}
	else if(isToken("Shape"))
		{
		/* Parse a shape node: */
		getNextToken();
		result=new ShapeNode(*this);
		}
	else if(isToken("Appearance"))
		{
		/* Parse an appearance node: */
		getNextToken();
		result=new AppearanceNode(*this);
		}
	else if(isToken("Material"))
		{
		/* Parse a material node: */
		getNextToken();
		result=new MaterialNode(*this);
		}
	else if(isToken("ImageTexture"))
		{
		/* Parse an image texture node: */
		getNextToken();
		result=new ImageTextureNode(*this);
		}
	else if(isToken("IndexedFaceSet"))
		{
		/* Parse an indexed face set node: */
		getNextToken();
		result=new IndexedFaceSetNode(*this);
		}
	else if(isToken("TextureCoordinate"))
		{
		/* Parse a texture coordinate node: */
		getNextToken();
		result=new TextureCoordinateNode(*this);
		}
	else if(isToken("Color"))
		{
		/* Parse a color node: */
		getNextToken();
		result=new ColorNode(*this);
		}
	else if(isToken("Normal"))
		{
		/* Parse a normal node: */
		getNextToken();
		result=new NormalNode(*this);
		}
	else if(isToken("Coordinate"))
		{
		/* Parse a coordinate node: */
		getNextToken();
		result=new CoordinateNode(*this);
		}
	else if(isToken("LOD"))
		{
		/* Parse a LOD node: */
		getNextToken();
		result=new LODNode(*this);
		}
	else if(isToken("Inline"))
		{
		/* Parse an inline node: */
		getNextToken();
		result=new InlineNode(*this);
		}
	else
		{
		/* Skip the node entirely: */
		std::cout<<"Skipping node of type "<<getToken()<<std::endl;
		
		/* Check for the opening brace: */
		getNextToken();
		if(!isToken("{"))
			Misc::throwStdErr("VRMLParser::getNextNode: Missing opening brace in node definition");
		getNextToken();
		
		/* Skip tokens until the matching closing brace: */
		unsigned int braceLevel=1;
		while(braceLevel>0)
			{
			if(isToken("{"))
				++braceLevel;
			else if(isToken("}"))
				--braceLevel;
			getNextToken();
			}
		
		result=0;
		}
	
	if(nodeName.toStr()[0]!='\0')
		{
		/* Store the node in the dictionary: */
		nodeDictionary.setEntry(NodeDictionary::Entry(nodeName,result));
		}
	
	return result;
	}
