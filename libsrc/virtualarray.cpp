//
#include "virtualarray.h"



CVirtualArray::~CVirtualArray(void)
{
	int lenx, leny, lenz;
	int i, j;
	lenx= 1<<(DDWORD_BITS - HBITS_POSITION);
	leny= 1<<MBITS_POSITION;
	lenz= 1<<MBITS_POSITION;
	for (i=0; i<lenx; i++){
		char ***px = m_pArray[i];
		if (px!=NULL){
			for (j=0; j<leny; j++){
				char **py = px[j];
				if (py!=NULL){
					/*
					for (k=0; k<lenz; k++){
						char *pz = py[k];
						if (pz!=NULL){
							delete [] pz;
						}
					}
					*/
					delete [] py;
				}
			}
			delete []px;
		}
	}
}


void CVirtualArray::Test(void)
{
	unsigned int i, K=1L<<18;
	unsigned int *p;

	for (i=0; i<K; i+=2){
		p = (unsigned int*)Malloc(sizeof(int));
//		p = new int[1];
		if (p==NULL){
			printf("Max allocation size is %d M", i>>18);
			return;
		}
		*p = i;
		SetElementPtr(i, p);
	}

	for (i=0; i<K; i+=2){
		p =(unsigned int*) GetElementPtr(i);
		if (*p != i){
			printf("Error in position %d\n", i);
		}
	}
	

}


int CVirtualArray::Statistic(void)
{

	int size=0;

/*
	int i, j;
	size = 8192;
	for (i=0; i<(1<<(32-HBITS_POSITION)); i++){
		if (m_pArray[i]){
			size+=_MBLOCK_SIZE_;
			for (j=0; j<1024; j++)
				if (m_pArray[i][j]) 
					size+=_MBLOCK_SIZE_;
		}
	}

	size += sizeof(CMemoryMgr);
	printf("total memory cost is about %fK\n", (float)(size/1024.0));
*/
	return size;
}
