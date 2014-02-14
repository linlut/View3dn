//======================================================================
//      FILENAME:       memmgr.cpp                                     //
//      PURPOSE:        VIRTUAL MEMORY ALLOCATION MANAGER IN DOS4GW    //
//      DATE:           Aug 20, 2007                                   //
//      AUTHOR:         Zhang Nan                                      //
//                    
//=======================================================================

#include <crest/base/sysconf.h>
#include <crest/base/memmgr.h>



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

	_CMemCB * next(void)
	{
		return m_pNext;
	}

	bool enough_space(const int size)
	{
		if (size<=m_nSize)
			return true;
		else
			return false;
	}

};


void CMemoryMgr::Init(void)
{
    for (int i=0; i<m_nBlockNum; i++) 
		m_pBlock[i] = NULL;
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
    int i;

    for (i=0; i<m_nBlockNum; i++){
		if (m_pBlock[i] != NULL ) 
			delete [] m_pBlock[i];
    }
	Init();
}


/**************************************************************
        function name:  ARAllocCell()
        input:          i-----handle, size----WORD
        output:         pointer to a block----char far *
**************************************************************/
char* CMemoryMgr::Malloc(unsigned int size)
{
	int tmp;

	if (size>_PAGESIZE_) return NULL;

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
			assert(m_pBlock[m_nCurWorkingBlcok]!=NULL);
			m_nOffsetInBlock = 0;
		}

		if ((m_nOffsetInBlock + size)<=_PAGESIZE_){
			tmp = m_nOffsetInBlock;
			m_nOffsetInBlock += size;
			return m_pBlock[m_nCurWorkingBlcok]+ tmp;
		}
		else{
			m_nCurWorkingBlcok++;
		}
	}
	printf("Out of memory block!!\n\n");
	fflush(stdout);
	return NULL;
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
    int i;
    
    px = (char****)(Malloc((nx * sizeof(char***))));
    if (!px) return NULL;

    for (i = 0; i< nx; i++){
		px[i] = Malloc3DArray(ny, nz, nw, sizeof_element);
		if (!px[i]) return NULL;
    }
    return px;
}


CMemoryMgr _zzzmem(3072);
