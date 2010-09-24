/***********************************************************************
RowColumn - Container class to arrange children on a two-dimensional
grid.
Copyright (c) 2001-2005 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Event.h>

#include <GLMotif/RowColumn.h>

namespace GLMotif {

/**************************
Methods of class RowColumn:
**************************/

Vector RowColumn::calcGrid(std::vector<GLfloat>& columnWidths,std::vector<GLfloat>& rowHeights) const
	{
	/* Calculate the natural size of the grid: */
	GLfloat maxWidth=0.0f;
	GLfloat maxHeight=0.0f;
	rowHeights.clear();
	int rowIndex=0;
	columnWidths.clear();
	int columnIndex=0;
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		{
		/* Fit the current child into its grid cell: */
		if(rowIndex==int(rowHeights.size()))
			rowHeights.push_back(0.0f);
		if(columnIndex==int(columnWidths.size()))
			columnWidths.push_back(0.0f);
		Vector childSize=(*chIt)->calcNaturalSize();
		if(rowHeights[rowIndex]<childSize[1])
			{
			rowHeights[rowIndex]=childSize[1];
			if(maxHeight<childSize[1])
				maxHeight=childSize[1];
			}
		if(columnWidths[columnIndex]<childSize[0])
			{
			columnWidths[columnIndex]=childSize[0];
			if(maxWidth<childSize[0])
				maxWidth=childSize[0];
			}
		
		/* Go to the next child: */
		if(orientation==VERTICAL)
			{
			if(++columnIndex==numMinorWidgets)
				{
				columnIndex=0;
				++rowIndex;
				}
			}
		else
			{
			if(++rowIndex==numMinorWidgets)
				{
				rowIndex=0;
				++columnIndex;
				}
			}
		}
	
	if(packing==PACK_GRID)
		{
		/* Set all sizes to the maximum: */
		for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
			*rIt=maxHeight;
		for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
			*cIt=maxWidth;
		}
	
	/* Calculate the overall size: */
	Vector result(0.0f,0.0f,0.0f);
	for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
		result[0]+=*cIt;
	for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
		result[1]+=*rIt;
	result[0]+=GLfloat(columnWidths.size()-1)*spacing;
	result[1]+=GLfloat(rowHeights.size()-1)*spacing;
	
	return result;
	}

RowColumn::RowColumn(const char* sName,Container* sParent,bool sManageChild)
	:Container(sName,sParent,false),
	 orientation(VERTICAL),
	 packing(PACK_TIGHT),
	 numMinorWidgets(1)
	{
	/* Get the style sheet: */
	const StyleSheet* ss=getStyleSheet();
	
	/* Set the children margin and spacing: */
	marginWidth=ss->containerMarginWidth;
	spacing=ss->containerSpacing;
	
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

RowColumn::~RowColumn(void)
	{
	/* Delete all children: */
	for(WidgetList::iterator chIt=children.begin();chIt!=children.end();++chIt)
		delete *chIt;
	}

Vector RowColumn::calcNaturalSize(void) const
	{
	/* Calculate the natural grid size: */
	std::vector<GLfloat> columnWidths,rowHeights;
	Vector result=calcGrid(columnWidths,rowHeights);
	result[0]+=2.0f*marginWidth;
	result[1]+=2.0f*marginWidth;
	
	return calcExteriorSize(result);
	}

ZRange RowColumn::calcZRange(void) const
	{
	/* Calculate the parent class widget's z range: */
	ZRange myZRange=Container::calcZRange();
	
	/* Calculate the combined z range of all children: */
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt)
		{
		ZRange childZRange=(*chIt)->calcZRange();
		if(myZRange.first>childZRange.first)
			myZRange.first=childZRange.first;
		if(myZRange.second<childZRange.second)
			myZRange.second=childZRange.second;
		}
	
	return myZRange;
	}

void RowColumn::resize(const Box& newExterior)
	{
	/* Resize the parent class widget: */
	Container::resize(newExterior);
	
	if(!children.empty())
		{
		/* Calculate the natural grid size: */
		std::vector<GLfloat> columnWidths,rowHeights;
		Vector gridSize=calcGrid(columnWidths,rowHeights);

		/* Adjust the grid size to the widget size: */
		Box box=getInterior().inset(Vector(marginWidth,marginWidth,0.0f));
		if(packing==PACK_TIGHT)
			{
			columnWidths[columnWidths.size()-1]+=box.size[0]-gridSize[0];
			rowHeights[rowHeights.size()-1]+=box.size[1]-gridSize[1];
			}
		else
			{
			GLfloat columnWidth=(box.size[0]-GLfloat(columnWidths.size()-1)*spacing)/GLfloat(columnWidths.size());
			for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
				*cIt=columnWidth;
			GLfloat rowHeight=(box.size[1]-GLfloat(rowHeights.size()-1)*spacing)/GLfloat(rowHeights.size());
			for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
				*rIt=rowHeight;
			}

		/* Fill the grid descriptor arrays: */
		Vector origin=box.origin;
		origin[1]+=box.size[1]+spacing;
		rows.clear();
		for(unsigned int i=0;i<rowHeights.size();++i)
			{
			origin[1]-=rowHeights[i]+spacing;
			rows.push_back(GridCell(origin[1],rowHeights[i]));
			}
		columns.clear();
		for(unsigned int i=0;i<columnWidths.size();++i)
			{
			columns.push_back(GridCell(origin[0],columnWidths[i]));
			origin[0]+=columnWidths[i]+spacing;
			}

		/* Position and size all children: */
		int rowIndex=0;
		int columnIndex=0;
		for(WidgetList::iterator chIt=children.begin();chIt!=children.end();++chIt)
			{
			/* Resize the current child: */
			(*chIt)->resize(Box(Vector(columns[columnIndex].origin,rows[rowIndex].origin,origin[2]),Vector(columnWidths[columnIndex],rowHeights[rowIndex],0.0f)));

			/* Go to the next child: */
			if(orientation==VERTICAL)
				{
				if(++columnIndex==numMinorWidgets)
					{
					columnIndex=0;
					++rowIndex;
					}
				}
			else
				{
				if(++rowIndex==numMinorWidgets)
					{
					rowIndex=0;
					++columnIndex;
					}
				}
			}
		}
	}

void RowColumn::draw(GLContextData& contextData) const
	{
	/* Draw the parent class widget: */
	Container::draw(contextData);
	
	/* Draw the margin and separators: */
	glColor(backgroundColor);
	Vector p=getInterior().origin;
	
	/* Draw the top left margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(0));
	p[0]=getInterior().origin[0]+marginWidth;
	for(int i=rows.size()-1;i>=0;--i)
		{
		p[1]=rows[i].origin;
		glVertex(p);
		p[1]+=rows[i].size;
		glVertex(p);
		}
	p[0]+=columns[0].size;
	glVertex(p);
	for(unsigned int i=1;i<columns.size();++i)
		{
		p[0]=columns[i].origin;
		glVertex(p);
		p[0]+=columns[i].size;
		glVertex(p);
		}
	glVertex(getInterior().getCorner(3));
	glEnd();
	
	/* Draw the bottom right margin part: */
	glBegin(GL_TRIANGLE_FAN);
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	for(unsigned int i=0;i<rows.size();++i)
		{
		p[1]=rows[i].origin+rows[i].size;
		glVertex(p);
		p[1]=rows[i].origin;
		glVertex(p);
		}
	p[0]=columns[columns.size()-1].origin;
	glVertex(p);
	for(int i=columns.size()-2;i>=0;--i)
		{
		p[0]=columns[i].origin+columns[i].size;
		glVertex(p);
		p[0]=columns[i].origin;
		glVertex(p);
		}
	glVertex(getInterior().getCorner(0));
	glEnd();
	
	/* Draw the horizontal separators: */
	for(unsigned int i=1;i<rows.size();++i)
		{
		GLfloat y0=rows[i-1].origin;
		GLfloat y1=rows[i].origin+rows[i].size;
		glBegin(GL_QUAD_STRIP);
		for(unsigned int j=0;j<columns.size();++j)
			{
			GLfloat x=columns[j].origin;
			glVertex3f(x,y0,p[2]);
			glVertex3f(x,y1,p[2]);
			x+=columns[j].size;
			glVertex3f(x,y0,p[2]);
			glVertex3f(x,y1,p[2]);
			}
		glEnd();
		}
	
	/* Draw the vertical separators: */
	glBegin(GL_QUADS);
	for(unsigned int i=0;i<rows.size();++i)
		{
		GLfloat y0=rows[i].origin;
		GLfloat y1=y0+rows[i].size;
		for(unsigned int j=1;j<columns.size();++j)
			{
			GLfloat x0=columns[j-1].origin+columns[j-1].size;
			GLfloat x1=columns[j].origin;
			glVertex3f(x0,y0,p[2]);
			glVertex3f(x1,y0,p[2]);
			glVertex3f(x1,y1,p[2]);
			glVertex3f(x0,y1,p[2]);
			}
		}
	glEnd();
	
	/* Draw the children: */
	int numChildren=0;
	for(WidgetList::const_iterator chIt=children.begin();chIt!=children.end();++chIt,++numChildren)
		(*chIt)->draw(contextData);
	
	/* Are there leftover unfilled cells? */
	int minorIndex=numChildren%numMinorWidgets;
	if(minorIndex!=0)
		{
		int majorIndex=numChildren/numMinorWidgets;
		const GridCell* row;
		const GridCell* col;
		if(orientation==VERTICAL)
			{
			col=&columns[minorIndex];
			row=&rows[majorIndex];
			}
		else
			{
			col=&columns[majorIndex];
			row=&rows[minorIndex];
			}
		for(;minorIndex<numMinorWidgets;++minorIndex)
			{
			/* Fill the hole: */
			glColor(backgroundColor);
			glBegin(GL_QUADS);
			glNormal3f(0.0f,0.0f,1.0f);
			glVertex3f(col->origin,row->origin,p[2]);
			glVertex3f(col->origin+col->size,row->origin,p[2]);
			glVertex3f(col->origin+col->size,row->origin+row->size,p[2]);
			glVertex3f(col->origin,row->origin+row->size,p[2]);
			glEnd();
			if(orientation==VERTICAL)
				++col;
			else
				++row;
			}
		}
	}

bool RowColumn::findRecipient(Event& event)
	{
	/* Distribute the question to the child widgets: */
	bool childFound=false;
	for(WidgetList::iterator chIt=children.begin();chIt!=children.end();++chIt)
		childFound|=(*chIt)->findRecipient(event);
	
	/* If no child was found, return ourselves (and ignore any incoming events): */
	if(childFound)
		return true;
	else
		{
		/* Check ourselves: */
		Event::WidgetPoint wp=event.calcWidgetPoint(this);
		if(isInside(wp.getPoint()))
			return event.setTargetWidget(this,wp);
		else
			return false;
		}
	}

void RowColumn::addChild(Widget* newChild)
	{
	/* Add the child to the list: */
	children.push_back(newChild);
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new child: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::requestResize(Widget* child,const Vector& newExteriorSize)
	{
	/* Just grant the request if nothing really changed: */
	if(!isManaged)
		child->resize(Box(child->getExterior().origin,newExteriorSize));
	else if(newExteriorSize[0]==child->getExterior().size[0]&&newExteriorSize[1]==child->getExterior().size[1])
		child->resize(child->getExterior());
	else
		{
		/* Calculate the natural grid size: */
		std::vector<GLfloat> columnWidths,rowHeights;
		calcGrid(columnWidths,rowHeights);

		/* Adjust the grid size for the new child size: */
		if(packing==PACK_GRID)
			{
			if(rowHeights[0]<newExteriorSize[1])
				{
				/* Adjust all rows: */
				for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
					*rIt=newExteriorSize[1];
				}
			if(columnWidths[0]<newExteriorSize[0])
				{
				/* Adjust all columns: */
				for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
					*cIt=newExteriorSize[0];
				}
			}
		else
			{
			/* Find the child in the children list: */
			int childIndex=0;
			for(WidgetList::iterator chIt=children.begin();*chIt!=child;++chIt,++childIndex)
				;
			int rowIndex,columnIndex;
			if(orientation==VERTICAL)
				{
				columnIndex=childIndex%numMinorWidgets;
				rowIndex=childIndex/numMinorWidgets;
				}
			else
				{
				rowIndex=childIndex%numMinorWidgets;
				columnIndex=childIndex/numMinorWidgets;
				}

			/* Adjust the cell's size, if necessary: */
			if(rowHeights[rowIndex]<newExteriorSize[1])
				rowHeights[rowIndex]=newExteriorSize[1];
			if(columnWidths[columnIndex]<newExteriorSize[0])
				columnWidths[columnIndex]=newExteriorSize[0];
			}

		/* Calculate the overall size: */
		Vector result(0.0f,0.0f,0.0f);
		for(std::vector<GLfloat>::iterator cIt=columnWidths.begin();cIt!=columnWidths.end();++cIt)
			result[0]+=*cIt;
		for(std::vector<GLfloat>::iterator rIt=rowHeights.begin();rIt!=rowHeights.end();++rIt)
			result[1]+=*rIt;
		result[0]+=2.0f*marginWidth+GLfloat(columnWidths.size()-1)*spacing;
		result[1]+=2.0f*marginWidth+GLfloat(rowHeights.size()-1)*spacing;
		
		/* Try to resize the widget: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

Widget* RowColumn::getFirstChild(void)
	{
	return children.front();
	}

Widget* RowColumn::getNextChild(Widget* child)
	{
	/* Search the given widget in the children list: */
	WidgetList::iterator childIt;
	for(childIt=children.begin();childIt!=children.end();++childIt)
		if(*childIt==child)
			{
			/* Return the child after the found one: */
			++childIt;
			break;
			}
	
	/* If the iterator is valid, return its content; otherwise, return null: */
	if(childIt!=children.end())
		return *childIt;
	else
		return 0;
	}

void RowColumn::setOrientation(RowColumn::Orientation newOrientation)
	{
	/* Set the orientation: */
	orientation=newOrientation;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setPacking(RowColumn::Packing newPacking)
	{
	/* Set the packing strategy: */
	packing=newPacking;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setNumMinorWidgets(GLsizei newNumMinorWidgets)
	{
	/* Set the number of minor widgets: */
	numMinorWidgets=newNumMinorWidgets;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setMarginWidth(GLfloat newMarginWidth)
	{
	/* Set the margin width: */
	marginWidth=newMarginWidth;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

void RowColumn::setSpacing(GLfloat newSpacing)
	{
	/* Set the spacing: */
	spacing=newSpacing;
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new setting: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

GLint RowColumn::getChildIndex(const Widget* child) const
	{
	GLint result;
	
	/* Find the child's index in the list: */
	WidgetList::const_iterator chIt;
	for(result=0,chIt=children.begin();chIt!=children.end()&&*chIt!=child;++result,++chIt)
		;
	if(chIt==children.end())
		result=-1;
	
	return result;
	}

void RowColumn::insertChild(GLint position,Widget* newChild)
	{
	/* Insert the child into the list: */
	WidgetList::iterator chIt=children.begin();
	for(GLint i=0;i<position&&chIt!=children.end();++i,++chIt)
		;
	children.insert(chIt,newChild);
	
	if(isManaged)
		{
		/* Try to resize the widget to accomodate the new child: */
		parent->requestResize(this,calcNaturalSize());
		}
	}

}
