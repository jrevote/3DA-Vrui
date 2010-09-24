/***********************************************************************
PriorityHeap - Implementation of a priority queue with a heap structure.
Copyright (c) 2003-2005 Oliver Kreylos

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

#ifndef MISC_PRIORITYHEAP_INCLUDED
#define MISC_PRIORITYHEAP_INCLUDED

#include <new>
#include <Misc/Utility.h>

namespace Misc {

template <class Type>
class StdComp // Functor to compare elements of an arbitrary type
	{
	/* Methods: */
	public:
	static bool lessEqual(const Type& v1,const Type& v2)
		{
		return v1<=v2;
		}
	};

template <class Content,class Comparison =StdComp<Content> >
class PriorityHeap
	{
	friend class Iterator;
	
	/* Embedded classes: */
	public:
	class Iterator
		{
		friend class PriorityHeap;
		
		/* Elements: */
		private:
		PriorityHeap* heap; // Pointer to heap being iterated through
		size_t index; // Index of element currently referenced
		
		/* Constructors and destructors: */
		public:
		Iterator(void)
			:heap(0)
			{
			}
		private:
		Iterator(PriorityHeap* sHeap,size_t sIndex)
			:heap(sHeap),index(sIndex)
			{
			}
		public:
		Iterator(const Iterator& source)
			:heap(source.heap),index(source.index)
			{
			}
		
		/* Methods: */
		Iterator& operator=(const Iterator& source)
			{
			heap=source.heap;
			index=source.index;
			return *this;
			}
		friend bool operator==(const Iterator& it1,const Iterator& it2)
			{
			return it1.heap==it2.heap&&it1.index==it2.index;
			}
		friend bool operator!=(const Iterator& it1,const Iterator& it2)
			{
			return it1.heap!=it2.heap||it1.index!=it2.index;
			}
		bool isFinished(void) const
			{
			return heap==0||index>=heap->numElements;
			}
		const Content& operator*(void) const
			{
			return heap->heap[index];
			}
		Content& operator*(void)
			{
			return heap->heap[index];
			}
		const Content* operator->(void) const
			{
			return heap->heap+index;
			}
		Content* operator->(void)
			{
			return heap->heap+index;
			}
		Iterator& operator++(void)
			{
			++index;
			return *this;
			}
		Iterator operator++(int)
			{
			Iterator result(*this);
			++index;
			return result;
			}
		Iterator& operator+=(size_t increment)
			{
			index+=increment;
			return *this;
			}
		Iterator operator+(size_t increment) const
			{
			return Iterator(heap,index+increment);
			}
		};
	
	/* Elements: */
	private:
	size_t allocSize; // Size of allocated heap array
	float growRate; // Rate the heap array grows at when running out of space
	void* memChunk; // Pointer to uninitialized memory
	size_t numElements; // Number of elements currently in heap
	Content* heap; // Pointer to heap array
	
	/* Private methods: */
	void reallocate(size_t newAllocSize)
		{
		/* Allocate a new memory chunk: */
		allocSize=newAllocSize;
		void* newMemChunk=new char[allocSize*sizeof(Content)];
		Content* newHeap=static_cast<Content*>(newMemChunk);
		
		/* Copy all entries from the old heap, then delete the old entries: */
		for(size_t i=0;i<numElements;++i)
			{
			new(&newHeap[i]) Content(heap[i]);
			heap[i].~Content();
			}
		
		/* Delete the old heap and use the new one: */
		delete[] static_cast<char*>(memChunk);
		memChunk=newMemChunk;
		heap=newHeap;
		}
	
	/* Constructors and destructors: */
	public:
	PriorityHeap(size_t sAllocSize =0,float sGrowRate =1.5) // Creates empty heap
		:allocSize(sAllocSize),growRate(sGrowRate),
		 memChunk(new char[allocSize*sizeof(Content)]),
		 numElements(0),heap(static_cast<Content*>(memChunk))
		{
		}
	PriorityHeap(const PriorityHeap& source)
		:allocSize(source.allocSize),growRate(source.growRate),
		 memChunk(new char[allocSize*sizeof(Content)]),
		 numElements(source.numElements),heap(static_cast<Content*>(memChunk))
		{
		/* Copy all entries from the source heap: */
		for(size_t i=0;i<numElements;++i)
			new(&heap[i]) Content(source.heap[i]);
		}
	~PriorityHeap(void)
		{
		/* Destroy all heap entries: */
		for(size_t i=0;i<numElements;++i)
			heap[i].~Content();
		
		delete[] static_cast<char*>(memChunk);
		}
	
	/* Methods: */
	PriorityHeap& operator=(const PriorityHeap& source)
		{
		if(this!=&source)
			{
			/* Destroy all heap entries: */
			for(size_t i=0;i<numElements;++i)
				heap[i].~Content();
			
			/* Allocate a new memory chunk if necessary: */
			if(allocSize!=source.allocSize)
				{
				delete[] memChunk;
				allocSize=source.allocSize;
				memChunk=new char[allocSize*sizeof(Content)];
				heap=static_cast<Content*>(memChunk);
				}
			growRate=source.growRate;
			
			/* Copy all elements from the source heap: */
			numElements=source.numElements;
			for(size_t i=0;i<numElements;++i)
				new(&heap[i]) Content(source.heap[i]);
			}
		
		return *this;
		}
	bool isEmpty(void) const
		{
		return numElements==0;
		}
	size_t getNumElements(void) const
		{
		return numElements;
		}
	Iterator begin(void)
		{
		return Iterator(this,0);
		}
	Iterator end(void)
		{
		return Iterator(this,numElements);
		}
	PriorityHeap& insert(const Content& newElement)
		{
		if(numElements==allocSize)
			reallocate(int(float(allocSize)*growRate)+1);
		
		/* Find the correct insertion position for the new element: */
		size_t insertionPos=numElements;
		size_t parent=(insertionPos-1)>>1;
		if(insertionPos==0||Comparison::lessEqual(heap[parent],newElement))
			{
			/* Copy the new element to the previous unused slot: */
			new(&heap[insertionPos]) Content(newElement);
			}
		else
			{
			/* Copy the parent to the previous unused slot: */
			new(&heap[insertionPos]) Content(heap[parent]);
			insertionPos=parent;
			while(insertionPos>0)
				{
				parent=(insertionPos-1)>>1;
				if(Comparison::lessEqual(heap[parent],newElement)) // Do the elements have to be swapped?
					break;
				heap[insertionPos]=heap[parent];
				insertionPos=parent;
				}
			
			/* Copy the new element to the insertion spot: */
			heap[insertionPos]=newElement;
			}
		
		/* Increase the number of stored elements: */
		++numElements;
		
		return *this;
		}
	const Content& getSmallest(void) const
		{
		return heap[0];
		}
	Content& getSmallest(void)
		{
		return heap[0];
		}
	PriorityHeap& reinsertSmallest(void)
		{
		/* Find the correct insertion position for the (changed) smallest element: */
		size_t insertionPos=0;
		while(true)
			{
			size_t child1=(insertionPos<<1)+1;
			size_t child2=(insertionPos<<1)+2;
			size_t minIndex=insertionPos;
			if(child1<numElements&&!Comparison::lessEqual(heap[minIndex],heap[child1]))
				minIndex=child1;
			if(child2<numElements&&!Comparison::lessEqual(heap[minIndex],heap[child2]))
				minIndex=child2;
			if(minIndex==insertionPos)
				break;
			Misc::swap(heap[insertionPos],heap[minIndex]);
			insertionPos=minIndex;
			}
		return *this;
		}
	PriorityHeap& removeSmallest(void)
		{
		/* Find the correct position to insert the last element: */
		size_t insertionPos=0;
		while(true)
			{
			size_t child1=(insertionPos<<1)+1;
			size_t child2=(insertionPos<<1)+2;
			size_t minIndex=numElements-1;
			if(child1<numElements&&!Comparison::lessEqual(heap[minIndex],heap[child1]))
				minIndex=child1;
			if(child2<numElements&&!Comparison::lessEqual(heap[minIndex],heap[child2]))
				minIndex=child2;
			if(minIndex==numElements-1)
				break;
			heap[insertionPos]=heap[minIndex];
			insertionPos=minIndex;
			}
		--numElements;
		heap[insertionPos]=heap[numElements];
		heap[numElements].~Content();
		
		return *this;
		}
	};

}

#endif
