/***********************************************************************
RefCounted - Base class for objects that use reference counting for
automatic destruction of unreferenced objects.
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

#ifndef REFCOUNTED_INCLUDED
#define REFCOUNTED_INCLUDED

class RefCounted
	{
	/* Elements: */
	private:
	unsigned int refCount; // Counter for references to this object
	
	/* Constructors and destructors: */
	public:
	RefCounted(void) // Creates an unreferenced object
		:refCount(0)
		{
		};
	RefCounted(const RefCounted& source) // Copy constructor; creates an unreferenced copy of the source object
		:refCount(0)
		{
		};
	RefCounted& operator=(const RefCounted& source) // Assignment operator; does not change the reference count of the changed object
		{
		};
	virtual ~RefCounted(void) // Destroys the object
		{
		};
	
	/* Methods: */
	void ref(void) // Adds another reference to the object
		{
		++refCount;
		};
	void unref(void) // Removes a reference from the object; destroys object when reference count reaches zero
		{
		--refCount;
		if(refCount==0)
			delete this;
		};
	};

#endif
