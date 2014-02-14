//======================================================================
//      FILENAME:       memmgr.cpp                                     //
//      PURPOSE:        VIRTUAL MEMORY ALLOCATION MANAGER IN DOS4GW    //
//      COMPILE:        flat                                           //
//      DATE:           Jan,5th   1997                                 //
//      Revision:       Dec. 29, 2010                                  //
//      AUTHOR:         Zhang Nan                                      //
//																	   //
//		Previous name armem.cpp                                        //
//=======================================================================

#include <sysconf.h>
#include "memmgr.h"


void CMemoryMgr::Init(void)
{
	for (int i=0; i<m_nBlockNum; i++) m_pBlock[i]=NULL;
	m_nCurWorkingBlcok =0;
	m_nOffsetInBlock = 0;
	m_pFreeSpace = NULL;
}


/************************************************************
  procedure to free all the memory blocks
  input ---none
************************************************************/
void CMemoryMgr::Free(void)
{
    for (int i=0; i<m_nBlockNum; i++){
		if (m_pBlock[i]){
			delete []m_pBlock[i];
			m_pBlock[i]=NULL;
		}
    }
	Init();
}

//
void CMemoryMgr::Mfree(void *ptr, unsigned int msize)
{
	_CMemCB *p = (_CMemCB *)ptr;
	p->m_pNext = (_CMemCB *)m_pFreeSpace;
	p->m_nSize = msize;
	m_pFreeSpace = (char*)p;
}


//alloc memory for a nx*ny*nz array using alloc_mem function
char*** CMemoryMgr::Malloc3DArray(const int nx, const int ny, const int nz, const int sizeof_element)
{
    char ***px;
    int i;
    
    px = (char ***)(Malloc(nx * sizeof(char**)));
    if (!px) return NULL;

    for (i = 0; i< nx; i++){
		px[i] = Malloc2DArray(ny, nz, sizeof_element);
		if (!px[i]) return NULL;
    }
    return px;
}


//alloc memory for a nx*ny*nz*nw array using alloc_mem function
char**** CMemoryMgr::Malloc4DArray(const int nx, const int ny, const int nz, const int nw, int sizeof_element)
{
    char ****px;    
    px = (char****)(Malloc((nx * sizeof(char***))));
    if (!px) return NULL;

    for (int i = 0; i< nx; i++){
		px[i] = Malloc3DArray(ny, nz, nw, sizeof_element);
		if (!px[i]) return NULL;
    }
    return px;
}


 CMemoryMgr _zzzmem(2048);
