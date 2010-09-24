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

#ifndef GLMOTIF_ROWCOLUMN_INCLUDED
#define GLMOTIF_ROWCOLUMN_INCLUDED

#include <vector>
#include <GLMotif/Container.h>

namespace GLMotif {

class RowColumn:public Container
	{
	/* Embedded classes: */
	public:
	enum Orientation // Type to select the major orientation of children layout
		{
		VERTICAL,HORIZONTAL
		};
	
	enum Packing // Type to select the packing of children
		{
		PACK_TIGHT,PACK_GRID
		};
	
	protected:
	struct GridCell // Structure to store origin and size of grid cells
		{
		/* Elements: */
		public:
		GLfloat origin,size; // Origin and size of a grid cell
		
		/* Constructors and destructors: */
		GridCell(GLfloat sOrigin,GLfloat sSize)
			:origin(sOrigin),size(sSize)
			{
			}
		};
	
	typedef std::vector<Widget*> WidgetList; // Data type for list of child widgets
	
	/* Elements: */
	protected:
	Orientation orientation; // Major layout orientation of children
	Packing packing; // Packing strategy
	GLsizei numMinorWidgets; // Number of widgets in minor layout orientation
	GLfloat marginWidth; // Width of margin around table
	GLfloat spacing; // Width of spacing between children
	std::vector<GridCell> rows,columns; // Grid cell descriptors
	WidgetList children; // List of child widgets
	
	/* Protected methods: */
	Vector calcGrid(std::vector<GLfloat>& columnWidths,std::vector<GLfloat>& rowHeights) const;
	
	/* Constructors and destructors: */
	public:
	RowColumn(const char* sName,Container* sParent,bool manageChild =true);
	virtual ~RowColumn(void);
	
	/* Methods inherited from Widget: */
	virtual Vector calcNaturalSize(void) const;
	virtual ZRange calcZRange(void) const;
	virtual void resize(const Box& newExterior);
	virtual void draw(GLContextData& contextData) const;
	virtual bool findRecipient(Event& event);
	
	/* Methods inherited from Container: */
	virtual void addChild(Widget* newChild);
	virtual void requestResize(Widget* child,const Vector& newExteriorSize);
	virtual Widget* getFirstChild(void);
	virtual Widget* getNextChild(Widget* child);
	
	/* New methods: */
	void setOrientation(Orientation newOrientation); // Changes the major layout orientation
	void setPacking(Packing newPacking); // Changes packing strategy
	void setNumMinorWidgets(GLsizei newNumMinorWidgets); // Changes the number of widgets in minor layout orientation
	void setMarginWidth(GLfloat newMarginWidth); // Changes the margin width
	void setSpacing(GLfloat newSpacing); // Changes the spacing
	virtual GLint getChildIndex(const Widget* child) const; // Returns the index of a radiobox child widget, -1 if not a child
	virtual void insertChild(GLint position,Widget* newChild); // Inserts new child at given position
	};

}

#endif
