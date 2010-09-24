/***********************************************************************
GLContextData - Class to store per-GL-context data for application
objects.
Copyright (c) 2000-2006 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLCONTEXTDATA_INCLUDED
#define GLCONTEXTDATA_INCLUDED

#include <vector>
#include <Misc/HashTable.h>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <GL/TLSHelper.h>
#include <GL/GLObject.h>

class GLContextData
	{
	/* Embedded classes: */
	public:
	struct CurrentContextDataChangedCallbackData:public Misc::CallbackData
		{
		/* Elements: */
		public:
		GLContextData* oldContext; // Old context data object
		GLContextData* newContext; // New context data object
		
		/* Constructors and destructors: */
		CurrentContextDataChangedCallbackData(GLContextData* sOldContext,GLContextData* sNewContext)
			:oldContext(sOldContext),newContext(sNewContext)
			{
			}
		};
	
	private:
	typedef Misc::HashTable<const GLObject*,GLObject::DataItem*> ItemHash; // Class for hash table mapping pointers to data items
	
	/* Elements: */
	static Misc::CallbackList currentContextDataChangedCallbacks; // List of callbacks called whenever the current context data object changes
	static GL_THREAD_LOCAL(GLContextData*) currentContextData; // Pointer to the current context data object (associated with the current OpenGL context)
	ItemHash context; // A hash table for the context
	
	/* Constructors and destructors: */
	public:
	GLContextData(int sTableSize,float sWaterMark =0.9f,float sGrowRate =1.7312543); // Constructs an empty context
	~GLContextData(void);
	
	/* Methods to manage object initializations and clean-ups: */
	static void initThing(const GLObject* thing); // Marks a thing for context initialization
	static void destroyThing(const GLObject* thing); // Marks a thing for context data removal
	static void resetThingManager(void); // Resets the thing manager
	void updateThings(void); // Initializes or deletes all marked things
	
	/* Methods to manage the current context: */
	static Misc::CallbackList& getCurrentContextDataChangedCallbacks(void) // Returns the list of callbacks called whenever the current context data object changes
		{
		return currentContextDataChangedCallbacks;
		}
	static GLContextData* getCurrent(void) // Returns the current context data object
		{
		return currentContextData;
		}
	static void makeCurrent(GLContextData* newCurrentContextData); // Sets the given context data object as the current one
	
	/* Methods to store/retrieve context data items: */
	bool isRealized(const GLObject* thing) const
		{
		return context.isEntry(thing);
		}
	void addDataItem(const GLObject* thing,GLObject::DataItem* dataItem)
		{
		context.setEntry(ItemHash::Entry(thing,dataItem));
		}
	template <class DataItemParam>
	DataItemParam* retrieveDataItem(const GLObject* thing)
		{
		/* Find the data item associated with the given thing: */
		ItemHash::Iterator dataIt=context.findEntry(thing);
		if(dataIt.isFinished())
			return 0;
		else
			{
			/* Cast the data item's pointer to the requested type and return it: */
			return dynamic_cast<DataItemParam*>(dataIt->getDest());
			}
		}
	void removeDataItem(const GLObject* thing)
		{
		/* Find the data item associated with the given thing: */
		ItemHash::Iterator dataIt=context.findEntry(thing);
		if(!dataIt.isFinished())
			{
			/* Delete the data item (hopefully freeing all resources): */
			delete dataIt->getDest();
			
			/* Remove the data item from the hash table: */
			context.removeEntry(dataIt);
			}
		}
	};

#endif
