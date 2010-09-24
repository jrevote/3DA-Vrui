/***********************************************************************
Pointer - Class for smart pointers to shared objects using reference
counting for object destruction.
Copyright (c) 2000-2008 Oliver Kreylos

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

#ifndef POINTER_INCLUDED
#define POINTER_INCLUDED

#include "RefCounted.h"

template <class ReferenceParam>
class Pointer
	{
	/* Embedded classes: */
	public:
	typedef ReferenceParam Reference; // Type of referenced objects
	
	/* Elements: */
	private:
	Reference* ref; // Pointer to the referenced object
	
	/* Constructors and destructors: */
	public:
	Pointer(void) // Creates a NULL pointer
		:ref(0)
		{
		};
	Pointer(Reference* sRef) // Creates a pointer to the given object
		:ref(sRef)
		{
		if(ref!=0)
			ref->ref();
		};
	Pointer(const Pointer& source) // Copy constructor
		:ref(source.ref)
		{
		if(ref!=0)
			ref->ref();
		};
	template <class SourceReferenceParam>
	Pointer(const Pointer<SourceReferenceParam>& source) // Copy constructor from base class using dynamic cast
		:ref(dynamic_cast<Reference*>(source.ptr()))
		{
		if(ref!=0)
			ref->ref();
		};
	Pointer& operator=(Reference* sRef) // Assignment operator to standard pointer
		{
		/* Detect aliasing: */
		if(ref!=sRef)
			{
			/* Unreference the old object: */
			if(ref!=0)
				ref->unref();
			
			/* Reference the new object: */
			ref=sRef;
			if(ref!=0)
				ref->ref();
			}
		};
	Pointer& operator=(const Pointer& source) // Assignment operator
		{
		/* Detect aliasing: */
		if(this!=&source)
			{
			/* Unreference the old object: */
			if(ref!=0)
				ref->unref();
			
			/* Reference the new object: */
			ref=source.ref;
			if(ref!=0)
				ref->ref();
			}
		};
	template <class SourceReferenceParam>
	Pointer& operator=(const Pointer<SourceReferenceParam>& source) // Assignment operator from base class using dynamic cast
		{
		/* Unreference the old object: */
		if(ref!=0)
			ref->unref();
		
		/* Reference the new object: */
		ref=dynamic_cast<Reference*>(source.ptr());
		if(ref!=0)
			ref->ref();
		};
	~Pointer(void) // Unreferences the object and potentially destroys it
		{
		if(ref!=0)
			ref->unref();
		};
	
	/* Methods: */
	friend bool operator==(const Pointer& p1,const Pointer& p2) // Equality operator
		{
		return p1.ref==p2.ref;
		};
	friend bool operator!=(const Pointer& p1,const Pointer& p2) // Inequality operator
		{
		return p1.ref!=p2.ref;
		};
	Reference* ptr(void) const // Converts smart pointer to dumb pointer
		{
		return ref;
		};
	Reference& operator*(void) const // Dereference operator
		{
		return *ref;
		};
	Reference* operator->(void) const // Structure access operator
		{
		return ref;
		};
	};

#endif
