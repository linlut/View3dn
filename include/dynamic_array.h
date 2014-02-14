//************************************************************
//
// dynamic_array.h
// Nan Zhang  Sep. 10, 2002
//
//************************************************************

#ifndef __CDynamicArray_H
#define __CDynamicArray_H

#include "sysconf.h"


#define _ARRAY_INITIAL_LENGTH_		32768
#define _ARRAY_INCREMENTAL_SIZE_	16384
#define _MINIMAL_SIZE_              512


class CWasteElement
{
public:
	unsigned char valid;	//valid flag;
	unsigned char pad1;		//padding bytes;
	unsigned char pad2;
	unsigned char pad3;
	int m_nNext;			//pointer to the next element 
};



template <class T> class CDynamicArray
{
private:
	int size;
	int space;
	int m_nArrayInitialLength;
	int m_nArrayIncrementalSize;
	T *elements;
	int m_nRecyleBuffer;

	void *alloc_space(const int array_length, const int element_size)
	{
		int msize = array_length*element_size;
		char *ptr = new char [msize];
		assert(ptr!=NULL);
		return (void*)ptr;
	}

	void free_space(void)
	{
		if (elements!=NULL){
			free(elements);
		}
	}


	// This function allocates more space internally;
	// to add elements call Add().
	void _allocate(const int atLeast) 
	{
		if (atLeast<= space)
			return;

		int newSpace = space+m_nArrayIncrementalSize;
		if (newSpace<atLeast) 
			newSpace = atLeast;

		T *newElements = (T*)alloc_space(newSpace, sizeof(T));
		memcpy(newElements, elements, sizeof(T)*space);
		free_space();

		elements = newElements;
		space = newSpace;
	}


public: 
	//constructor and deconstructor
	CDynamicArray(	const int arraylen=_ARRAY_INITIAL_LENGTH_, 
					const int incsize=_ARRAY_INCREMENTAL_SIZE_) 
	{
		m_nArrayInitialLength = arraylen;
		m_nArrayIncrementalSize = incsize;
		elements = (T*)alloc_space(m_nArrayInitialLength, sizeof(T));
		space = m_nArrayInitialLength;
		size = 0;
		m_nRecyleBuffer = -1L;
	}


	~CDynamicArray() 
	{
		free_space();
		elements=NULL;
	}

	//copy another array
	CDynamicArray(const CDynamicArray<T> & other)
	{
		int n;
		m_nArrayInitialLength = _ARRAY_INITIAL_LENGTH_;
		m_nArrayIncrementalSize = _ARRAY_INCREMENTAL_SIZE_;
		elements = NULL;
		size = 0;
		space = 0;
		n = other.GetSize(); 
		_allocate(n);
		for (int i=0; i<n; i++)
			elements[i]=other.elements[i];
		m_nRecyleBuffer = other.m_nRecyleBuffer;
	}

	void Append(const CDynamicArray<T> & other, int len) 
	{
		int l;

		l= other.GetSize();
		if (l>len) l=len;
		_allocate(l+size);
		for (int i=0; i<other.GetSize(); i++)
			Add(other.elements[i]);
	}

	CDynamicArray<T> & operator =(const CDynamicArray<T> & other) 
	{
		//
		free_space();

		//
		elements = alloc_space(1, sizeof(T));
		size = 0;
		space = 1;
		_allocate(other.GetSize());
		for (int i=0; i<other.GetSize(); i++)
			Add(other.elements[i]);
		return *this;
	}

	//insert elements
	T* Add(const T & newElement) 
	{
		T * ptr = NULL;

		//if the recycle buffer is not empty, use the waste buffer first;
		if (m_nRecyleBuffer!=-1){
			int index = m_nRecyleBuffer;
			CWasteElement *pwaste = (CWasteElement*)(&elements[index]);
			ptr = (T*)pwaste;
			m_nRecyleBuffer = pwaste->m_nNext;
			elements[index] = newElement;
		}
		else{
			_allocate(size+1);
			ptr = &elements[size];
			*ptr = newElement;
			size++;
		}
		return ptr;
	}


	void Recycle(T* oldElement) 
	{
		CWasteElement *pwaste = (CWasteElement*) oldElement;
		int offset = (unsigned int)oldElement - (unsigned int)elements;
		offset /= sizeof(T);
		pwaste->m_nNext = m_nRecyleBuffer;
		pwaste->valid = false;
		m_nRecyleBuffer = offset;
	}


	//insert elements
	void AddWithNoDuplication(const T & newElement) 
	{
		for (int i=0; i<size; i++){
			if (newElement==elements[i])
				return;
		}
		Add(newElement);
	}

	void Insert(const T & newElement, const int index) 
	{
		ASSERT0(m_nRecyleBuffer==-1);
		_allocate(size+5);
		if (index>size) {
			std::cerr << "error inserting new element at index " << index;
			std::cerr << " when size is " << size << std::endl;
			return;
		}
		for (int i=size-1; i>=index; i--) {
			elements[i+1] = elements[i];
		}
		elements[index] = newElement;
		size++;
	}

	void Remove(const int index) 
	{
		if (index < 0 || index >=size) {
			std::cerr << "Attempted to remove an element outside valid range!" << std::endl;
			std::cerr << "Attempted index " << index << " when size is ";
			std::cerr << size << "." << std::endl;
			return;
		}
		for (int i=index; i<size-1; i++)
			elements[i] = elements[i+1];
		size--;
	}

	void Resize(const int newsize)
	{
		size = newsize;
	}

	//direct access elements
	T & operator [](const int index) 
	{
#ifdef _DEBUG
	  if (index < 0 || index > size-1) {
		  std::cerr << "Attempted to index an array element outside valid range!";
		  std::cerr << "\nAttempted index " << index << " when size is ";
		  std::cerr << size << ".\n";
	  }
#endif
	  return elements[index];
	}

	int GetSize() const 
	{
		return size;
	}

	void RemoveAll()
	{
		size = 0;
	}

	T * GetBuffer(void)	{ return elements; }

	void toString(FILE *fp)
	{
		for (int i=0; i<size; i++){
			elements[i].toString(fp);
		}
	}

};

#endif // __CDynamicArray_H

