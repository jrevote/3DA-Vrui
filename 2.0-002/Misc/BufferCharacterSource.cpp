/***********************************************************************
BufferCharacterSource - High-performance ASCII file reader for memory
buffers.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Misc/BufferCharacterSource.h>

namespace Misc {

/**************************************
Methods of class BufferCharacterSource:
**************************************/

void BufferCharacterSource::fillBuffer(void)
	{
	/* Dummy function; never called */
	}

BufferCharacterSource::BufferCharacterSource(const void* sBuffer,size_t sBufferSize)
	:CharacterSource(0)
	{
	/* Override the character source's buffer: */
	bufferSize=sBufferSize;
	delete[] allocBuffer;
	allocBuffer=const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(sBuffer));
	buffer=allocBuffer;
	bufferEnd=buffer+bufferSize;
	eofPtr=bufferEnd;
	rPtr=buffer;
	}

BufferCharacterSource::~BufferCharacterSource(void)
	{
	/* Unattach the character source's buffer (since it's not owned by the character source): */
	allocBuffer=0;
	}

}
