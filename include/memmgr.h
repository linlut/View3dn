/*********************************************************
 * Header file for dynamic memory management
 * File name: memmgr.h
 * Auther: Nan Zhang
 * Date: Aug 20, 2008
 *********************************************************/
#ifndef __INC_MEMMGR_H__

#define __INC_MEMMGR_H__
#include <assert.h>
#include "sysconf.h"

#define _PAGESIZE_        (65500L)   
#define _BLOCK_NUM_       (1024*8L)


class _CMemCB{
public:
	int m_nSize;
	_CMemCB *m_pNext;

public:
	_CMemCB( const void *p, const int n)
	{
		int m;

		if (n<sizeof(_CMemCB)){
			printf("Allocate space is too small, adjusted!\n");
			m = sizeof(_CMemCB);
		}
		else
			m = n;
		m_nSize = m;
		m_pNext = (_CMemCB *)p;
	}

	~_CMemCB(){}

	inline _CMemCB * next(void)
	{
		return m_pNext;
	}

	inline bool enough_space(const int size)
	{
		if (size<=m_nSize)
			return true;
		else
			return false;
	}
};


//definition of memory structures
class CMemoryMgr
{
private:
	char** m_pBlock;			//pointer to the page buffer
	char* m_pFreeSpace;			//head pointer to the free space

	int m_nCurWorkingBlcok;		//current page pointer
	int m_nOffsetInBlock;		//offset in the page
	int m_nBlockNum;			//number of total pages
	//
	void Init(void);

public:
	//Clear all buffers
	void Free(void);

	//Constructor
	CMemoryMgr(int blocknum=_BLOCK_NUM_)
	{	
		if (blocknum<1) 
			blocknum = 1;
		else if (blocknum>_BLOCK_NUM_)
			blocknum = _BLOCK_NUM_;
		m_nBlockNum = blocknum;
		m_pBlock = new char *[m_nBlockNum];
		assert(m_pBlock!=NULL);
		Init();
	}

	//Destructor
	~CMemoryMgr()
	{
		Free();
		SafeDeleteArray(m_pBlock);
	}

	//Allocate a small amount of the memory space
	inline char* Malloc(unsigned int size)
	{
#ifdef _DEBUG
		if (size>_PAGESIZE_) return NULL;
#endif
		if (m_pFreeSpace!=NULL){
			_CMemCB *p = (_CMemCB *)m_pFreeSpace; 
			if (p->enough_space(size)){
				char *q = m_pFreeSpace;
				m_pFreeSpace = (char*)p->next();
				return q;
			}
		}
		while( m_nCurWorkingBlcok<m_nBlockNum ){
			if (m_pBlock[m_nCurWorkingBlcok]==NULL){
				m_pBlock[m_nCurWorkingBlcok] = new char[_PAGESIZE_];
				ASSERT0(m_pBlock[m_nCurWorkingBlcok]!=NULL);
				m_nOffsetInBlock = 0;
			}
			if ((m_nOffsetInBlock + size)<=_PAGESIZE_){
				int tmp = m_nOffsetInBlock;
				m_nOffsetInBlock += size;
				return m_pBlock[m_nCurWorkingBlcok]+ tmp;
			}
			else{
				m_nCurWorkingBlcok++;
			}
		}
		static char msg[]="You have reserved only %d pages, out of page error!\n";
		printf(msg, m_nBlockNum);
		return NULL;
	}

	//Free a small amount of the memory space
	void Mfree(void *ptr, unsigned int msize);

	//alloc memory for a nx*ny array using alloc_mem function
	inline char** Malloc2DArray(const int nx, const int ny, const int sizeof_element)
	{
		char **px;
		int i, size;    

		size = nx * sizeof(char*);
		if (!(px = (char **)(Malloc(size))))
			return NULL;
		size = ny * sizeof_element;
		for (i = 0; i< nx; i++)
			if (!(px[i] = Malloc(size))) 
				return NULL;
		return px;
	}

	//alloc memory for a nx*ny*nz array using alloc_mem function
	char*** Malloc3DArray(const int nx, const int ny, const int nz, const int sizeof_element);

	//alloc memory for a nx*ny*nz*nw array using alloc_mem function
	char**** Malloc4DArray(int nx, int ny, int nz, int nw, const int sizeof_element);
};



inline char * zmalloc(const int msize)
{
	extern CMemoryMgr _zzzmem;
	char *p=_zzzmem.Malloc(msize);
	ASSERT0(p!=NULL);
	return p;
}

inline void zfree(void)
{
	extern CMemoryMgr _zzzmem;
	_zzzmem.Free();
}


#endif