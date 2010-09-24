/***********************************************************************
IndexedFaceSetNode - Class for shapes represented as sets of faces.
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

#include <vector>
#include <Misc/HashTable.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/GLGeometryWrappers.h>

#include "SFBool.h"
#include "MFInt32.h"
#include "VRMLParser.h"
#include "TextureCoordinateNode.h"
#include "ColorNode.h"
#include "NormalNode.h"
#include "CoordinateNode.h"

#include "IndexedFaceSetNode.h"

namespace {

/**************
Helper classes:
**************/

struct VertexIndices // Structure to store the indices of vertex components
	{
	/* Elements: */
	public:
	int texCoord,color,normal,coord;
	
	/* Constructors and destructors: */
	VertexIndices(int sTexCoord,int sColor,int sNormal,int sCoord)
		:texCoord(sTexCoord),color(sColor),normal(sNormal),coord(sCoord)
		{
		};
	
	/* Methods: */
	friend bool operator==(const VertexIndices& vi1,const VertexIndices& vi2)
		{
		return vi1.texCoord==vi2.texCoord&&vi1.color==vi2.color&&vi1.normal==vi2.normal&&vi1.coord==vi2.coord;
		};
	friend bool operator!=(const VertexIndices& vi1,const VertexIndices& vi2)
		{
		return vi1.texCoord!=vi2.texCoord||vi1.color!=vi2.color||vi1.normal!=vi2.normal||vi1.coord!=vi2.coord;
		};
	static size_t hash(const VertexIndices& value,size_t tableSize)
		{
		return (((size_t(value.texCoord)*7+size_t(value.color))*5+size_t(value.normal)*3)+size_t(value.coord))%tableSize;
		};
	};

}

/*********************************************
Methods of class IndexedFaceSetNode::DataItem:
*********************************************/

IndexedFaceSetNode::DataItem::DataItem(void)
	:vertexBufferObjectId(0),
	 indexBufferObjectId(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create vertex and index buffer objects: */
		glGenBuffersARB(1,&vertexBufferObjectId);
		glGenBuffersARB(1,&indexBufferObjectId);
		}
	}

IndexedFaceSetNode::DataItem::~DataItem(void)
	{
	/* Destroy the buffer objects: */
	glDeleteBuffersARB(1,&vertexBufferObjectId);
	glDeleteBuffersARB(1,&indexBufferObjectId);
	}

/***********************************
Methods of class IndexedFaceSetNode:
***********************************/

IndexedFaceSetNode::IndexedFaceSetNode(VRMLParser& parser)
	:VRMLNode(parser),
	 ccw(true),
	 solid(true),
	 colorPerVertex(true),
	 normalPerVertex(true)
	{
	/* Check for the opening brace: */
	if(!parser.isToken("{"))
		Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: Missing opening brace in node definition");
	parser.getNextToken();
	
	/* Process attributes until closing brace: */
	while(!parser.isToken("}"))
		{
		if(parser.isToken("ccw"))
			{
			parser.getNextToken();
			ccw=SFBool::parse(parser);
			}
		else if(parser.isToken("solid"))
			{
			parser.getNextToken();
			solid=SFBool::parse(parser);
			}
		else if(parser.isToken("colorPerVertex"))
			{
			parser.getNextToken();
			colorPerVertex=SFBool::parse(parser);
			}
		else if(parser.isToken("normalPerVertex"))
			{
			parser.getNextToken();
			normalPerVertex=SFBool::parse(parser);
			}
		else if(parser.isToken("texCoord"))
			{
			/* Parse the texture coordinate node: */
			parser.getNextToken();
			texCoord=parser.getNextNode();
			}
		else if(parser.isToken("color"))
			{
			/* Parse the color node: */
			parser.getNextToken();
			color=parser.getNextNode();
			}
		else if(parser.isToken("normal"))
			{
			/* Parse the normal node: */
			parser.getNextToken();
			normal=parser.getNextNode();
			}
		else if(parser.isToken("coord"))
			{
			/* Parse the coordinate node: */
			parser.getNextToken();
			coord=parser.getNextNode();
			}
		else if(parser.isToken("texCoordIndex"))
			{
			/* Parse the texture coordinate index array: */
			parser.getNextToken();
			texCoordIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("colorIndex"))
			{
			/* Parse the color index array: */
			parser.getNextToken();
			colorIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("normalIndex"))
			{
			/* Parse the normal vector index array: */
			parser.getNextToken();
			normalIndices=MFInt32::parse(parser);
			}
		else if(parser.isToken("coordIndex"))
			{
			/* Parse the coordinate index array: */
			parser.getNextToken();
			coordIndices=MFInt32::parse(parser);
			}
		else
			Misc::throwStdErr("IndexedFaceSetNode::IndexedFaceSetNode: unknown attribute \"%s\" in node definition",parser.getToken());
		}
	
	/* Skip the closing brace: */
	parser.getNextToken();
	}

IndexedFaceSetNode::~IndexedFaceSetNode(void)
	{
	}

void IndexedFaceSetNode::initContext(GLContextData& contextData) const
	{
	/* Create a data item and store it in the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Do nothing if the vertex buffer object extension is not supported: */
	if(dataItem->vertexBufferObjectId==0||dataItem->indexBufferObjectId==0)
		return;
	
	const TextureCoordinateNode* texCoordNode=dynamic_cast<const TextureCoordinateNode*>(texCoord.ptr());
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.ptr());
	const NormalNode* normalNode=dynamic_cast<const NormalNode*>(normal.ptr());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.ptr());
	
	/*********************************************************************
	The problem with indexed face sets in VRML is that the format supports
	component-wise vertex indices, i.e., a vertex used in a face can have
	different indices for texture coordinate, color, normal, and position.
	OpenGL, on the other hand, only supports a single index for all vertex
	components. This method tries to reuse vertices as much as possible,
	by mapping tuples of per-component vertex indices to complete OpenGL
	vertex indices using a hash table.
	*********************************************************************/
	
	/* Create a hash table to map compound vertex indices to complete vertices: */
	typedef Misc::HashTable<VertexIndices,GLuint,VertexIndices> VertexHasher;
	VertexHasher vertexHasher(101);
	
	/* Count the number of vertices that need to be created and store their compound indices: */
	std::vector<int>::const_iterator texCoordIt=texCoordIndices.empty()?coordIndices.begin():texCoordIndices.begin();
	std::vector<int>::const_iterator colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
	std::vector<int>::const_iterator normalIt=normalIndices.empty()?coordIndices.begin():normalIndices.begin();
	std::vector<int>::const_iterator coordIt=coordIndices.begin();
	VertexIndices currentVertex(0,0,0,0);
	std::vector<VertexIndices> vertexIndices;
	GLuint arrayIndex=0;
	dataItem->numTriangles=0;
	while(coordIt!=coordIndices.end())
		{
		/* Process the vertices of this face: */
		size_t numFaceVertices=0;
		while(coordIt!=coordIndices.end()&&*coordIt>=0)
			{
			/* Create the current compound vertex: */
			if(texCoordNode!=0)
				currentVertex.texCoord=*texCoordIt;
			if(colorNode!=0)
				currentVertex.color=*colorIt;
			if(normalNode!=0)
				currentVertex.normal=*normalIt;
			currentVertex.coord=*coordIt;
			
			/* Find the index of the complete vertex: */
			if(!vertexHasher.isEntry(currentVertex))
				{
				vertexHasher.setEntry(VertexHasher::Entry(currentVertex,arrayIndex));
				vertexIndices.push_back(currentVertex);
				++arrayIndex;
				}
			
			/* Go to the next vertex in the same face: */
			++texCoordIt;
			if(colorPerVertex)
				++colorIt;
			if(normalPerVertex)
				++normalIt;
			++coordIt;
			++numFaceVertices;
			}
		
		/* Increment the total number of triangles: */
		dataItem->numTriangles+=numFaceVertices-2;
		
		/* Go to the next face: */
		++texCoordIt;
		++colorIt;
		++normalIt;
		++coordIt;
		}
	
	/* Upload all vertices into the vertex buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,size_t(arrayIndex)*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vertices=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(std::vector<VertexIndices>::const_iterator viIt=vertexIndices.begin();viIt!=vertexIndices.end();++viIt,++vertices)
		{
		/* Assemble the vertex from its components: */
		if(texCoordNode!=0)
			vertices->texCoord=Vertex::TexCoord(texCoordNode->getPoint(viIt->texCoord).getComponents());
		if(colorNode!=0)
			vertices->color=colorNode->getColor(viIt->color);
		if(normalNode!=0)
			vertices->normal=Vertex::Normal(normalNode->getVector(viIt->normal).getComponents());
		vertices->position=Vertex::Position(coordNode->getPoint(viIt->coord).getComponents());
		}
	
	/* Unmap and protect the vertex buffer: */
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	
	/* Upload all vertex indices into the index buffers: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->numTriangles*3*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
	GLuint* indices=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	
	texCoordIt=texCoordIndices.empty()?coordIndices.begin():texCoordIndices.begin();
	colorIt=colorIndices.empty()?coordIndices.begin():colorIndices.begin();
	normalIt=normalIndices.empty()?coordIndices.begin():normalIndices.begin();
	coordIt=coordIndices.begin();
	currentVertex=VertexIndices(0,0,0,0);
	while(coordIt!=coordIndices.end())
		{
		/* Process the vertices of this face: */
		GLuint triangleIndices[2];
		size_t numFaceVertices=0;
		while(coordIt!=coordIndices.end()&&*coordIt>=0)
			{
			/* Create the current compound vertex index: */
			if(texCoordNode!=0)
				currentVertex.texCoord=*texCoordIt;
			if(colorNode!=0)
				currentVertex.color=*colorIt;
			if(normalNode!=0)
				currentVertex.normal=*normalIt;
			currentVertex.coord=*coordIt;
			
			/* Get the array index of the assembled vertex: */
			GLuint index=vertexHasher.getEntry(currentVertex).getDest();
			
			/* Assemble triangles: */
			if(numFaceVertices<2)
				triangleIndices[numFaceVertices]=index;
			else
				{
				for(int i=0;i<2;++i)
					indices[i]=triangleIndices[i];
				indices[2]=index;
				triangleIndices[1]=index;
				indices+=3;
				}
			
			/* Go to the next vertex in the same face: */
			++texCoordIt;
			if(colorPerVertex)
				++colorIt;
			if(normalPerVertex)
				++normalIt;
			++coordIt;
			++numFaceVertices;
			}
		
		/* Go to the next face: */
		++texCoordIt;
		++colorIt;
		++normalIt;
		++coordIt;
		}
	
	/* Unmap and protect the index buffer: */
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

VRMLNode::Box IndexedFaceSetNode::calcBoundingBox(void) const
	{
	/* Get a pointer to the coord node: */
	CoordinateNode* coordNode=dynamic_cast<CoordinateNode*>(coord.ptr());
	
	/* Calculate the bounding box of all used vertex coordinates: */
	Box result=Box::empty;
	for(std::vector<int>::const_iterator ciIt=coordIndices.begin();ciIt!=coordIndices.end();++ciIt)
		if(*ciIt>=0)
			result.addPoint(coordNode->getPoint(*ciIt));
	return result;
	}

void IndexedFaceSetNode::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve the data item from the context: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	const TextureCoordinateNode* texCoordNode=dynamic_cast<const TextureCoordinateNode*>(texCoord.ptr());
	const ColorNode* colorNode=dynamic_cast<const ColorNode*>(color.ptr());
	const NormalNode* normalNode=dynamic_cast<const NormalNode*>(normal.ptr());
	const CoordinateNode* coordNode=dynamic_cast<const CoordinateNode*>(coord.ptr());
	
	/* Set up OpenGL: */
	if(ccw)
		glFrontFace(GL_CCW);
	else
		glFrontFace(GL_CW);
	if(solid)
		{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
		}
	else
		{
		glDisable(GL_CULL_FACE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		}
	
	if(dataItem->vertexBufferObjectId!=0&&dataItem->indexBufferObjectId!=0)
		{
		/* Determine which parts of the vertex array to enable: */
		int vertexPartsMask=0;
		if(texCoordNode!=0)
			vertexPartsMask|=GLVertexArrayParts::TexCoord;
		if(colorNode!=0)
			vertexPartsMask|=GLVertexArrayParts::Color;
		if(normalNode!=0)
			vertexPartsMask|=GLVertexArrayParts::Normal;
		vertexPartsMask|=GLVertexArrayParts::Position;
		
		/* Draw the indexed triangle set: */
		GLVertexArrayParts::enable(vertexPartsMask);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferObjectId);
		glVertexPointer(vertexPartsMask,static_cast<const Vertex*>(0));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferObjectId);
		glDrawElements(GL_TRIANGLES,dataItem->numTriangles*3,GL_UNSIGNED_INT,static_cast<const GLuint*>(0));
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
		GLVertexArrayParts::disable(vertexPartsMask);
		}
	else
		{
		/* Process all faces: */
		std::vector<int>::const_iterator texCoordIt=texCoordIndices.begin();
		std::vector<int>::const_iterator colorIt=colorIndices.begin();
		std::vector<int>::const_iterator normalIt=normalIndices.begin();
		std::vector<int>::const_iterator coordIt=coordIndices.begin();
		while(coordIt!=coordIndices.end())
			{
			glBegin(GL_POLYGON);
			while(*coordIt>=0)
				{
				if(texCoordNode!=0)
					glTexCoord(texCoordNode->getPoint(*texCoordIt));
				if(colorNode!=0)
					glColor(colorNode->getColor(*colorIt));
				if(normalNode!=0)
					glNormal(normalNode->getVector(*normalIt));
				glVertex(coordNode->getPoint(*coordIt));
				++texCoordIt;
				if(colorPerVertex)
					++colorIt;
				if(normalPerVertex)
					++normalIt;
				++coordIt;
				}
			glEnd();
			
			++texCoordIt;
			++colorIt;
			++normalIt;
			++coordIt;
			}
		}
	}
