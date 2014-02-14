//FILE: ViewSlicer.cpp
#include <crest/ogl/ViewSlicer.h>

namespace cia3d{
namespace ogl{


void ViewSlicer::_computeDistance2View(const void *_vert, const int floatsize, const int nv, float &distMin, float &distMax)
{
	//Manage the buffer
	if (m_nDistArray==0){
		m_pDistArray = new float[nv];
		m_nDistArray = nv;
		assert(m_pDistArray!=NULL);
	}
	else if (m_nDistArray<nv){
		SafeDeleteArray(m_pDistArray);
		m_pDistArray = new float[nv];
		m_nDistArray = nv;
	}

	const float dist1 = DotProd(m_viewdir, m_viewpos);
	distMin = 1e32f;
	distMax = -distMin;
	if (floatsize==8){//double
		const Vector3d *vert = (const Vector3d*)_vert;
		for (int i=0; i<nv; i++){
			const Vector3d * _p = &vert[i];
			Vector3f v((float)_p->x, (float)_p->y, (float)_p->z);
			const float dist0 = DotProd(m_viewdir, v);
			register float dd = dist0 - dist1;
			distMin = _MIN_(dd, distMin);
			distMax = _MAX_(dd, distMax);
			m_pDistArray[i] = dd;
		}
	}
	else{//float
		assert(floatsize==4); 
		const Vector3f *vert = (const Vector3f*)_vert;
		for (int i=0; i<nv; i++){
			const Vector3f v = vert[i];
			const float dist0 = DotProd(m_viewdir, v);
			register float dd = dist0 - dist1;
			distMin = _MIN_(dd, distMin);
			distMax = _MAX_(dd, distMax);
			m_pDistArray[i] = dd;
		}
	}
}


inline void 
getElementNearFar(float *pDistArray, const int *pelm, const int elmID, const int elmtype, float& distmin, float& distmax)
{
	distmin=1e32f;
	distmax=-distmin;	
	if (elmtype==4){
		const Vector4i* ptet = (const Vector4i*)pelm;
		const Vector4i tet = ptet[elmID];
		float t = pDistArray[tet.x];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[tet.y];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[tet.z];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[tet.w];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
	}
	else{
		const Vector8i* phex = (const Vector8i*)pelm;
		const Vector8i hex = phex[elmID];
		float t = pDistArray[hex.x];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.y];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.z];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.w];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);

		t = pDistArray[hex.x1];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.y1];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.z1];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
		t = pDistArray[hex.w1];
		distmin = _MIN_(distmin, t);
		distmax = _MAX_(distmax, t);
	}
}


void ViewSlicer::_bucketSortElements(const int *pelm, const int nelm, const int elmtype, const int direction, const float DMIN, const float DMAX)
{
	float dh1 = 1.0f/m_dh;
	if (m_pListNodeBuffer==NULL){
		m_pListNodeBuffer = new ListNode[nelm+10];
		assert(m_pListNodeBuffer!=NULL);
	}
	m_nBufferCount = 0;

	if (direction>0){ //Sort inceasingly
		for (int i=0; i<nelm; i++){
			float distmin, distmax;
			getElementNearFar(m_pDistArray, pelm, i, elmtype, distmin, distmax);
			//====================================
			int index = int((distmin-DMIN)*dh1+0.5f);
			if (index<0) index=0;
			int depth = int((distmax-DMIN)*dh1);
			if (depth<index) depth=index;
			//===========insert list=============
			ListNode *pnode = Malloc();
			pnode->data.depth = depth;
			pnode->data.elementID = i;
			m_listHead[index] = InsertAheadLinkedListNode(m_listHead[index], pnode);
		}
	}
	else{
		for (int i=0; i<nelm; i++){
			float distmin, distmax;
			getElementNearFar(m_pDistArray, pelm, i, elmtype, distmin, distmax);
			const int index = int((DMAX-distmax)*dh1+0.5f);
			int depth = int((DMAX-distmin)*dh1);
			if (depth<index) depth=index;
			//===========insert list=============
			ListNode *pnode = Malloc();
			pnode->data.depth = depth;
			pnode->data.elementID = i;
			m_listHead[index] = InsertAheadLinkedListNode(m_listHead[index], pnode);
		}
	}
}


float ViewSlicer::bucketSort(const void *vert, const int floatsize, const int nv, const int *pelm, const int nelm, const int elmtype, const int direction)
{
	int i;
	float distMin, distMax, rtval;

	m_nCounter++;
	m_pActiveList = NULL;

	//compute distance
	_computeDistance2View(vert, floatsize, nv, distMin, distMax);
	printf("Object Distance near: %g, far: %g\n", distMin, distMax);
	if (distMin<0) distMin=0;
			
	//======init bucket==================
	ASSERT0(m_dh>0);
	const float distlimit = fabs(distMax-distMin)*0.25;
	const float half_dh = _MIN_(m_dh*0.25, distlimit);
	int nSlice = (int)((distMax - distMin)/m_dh + 0.5f);
	if (nSlice<1) nSlice=1;
	if (nSlice>=VIEWSLICER_BUCKET_LENGTH-1){
		printf("Error: bucket buffer is too small! Size %d needed\n", nSlice);
		exit(0);
	}
	//Report number of slices
	printf("Slice number is %d\n", nSlice);
	
	m_nSlice = nSlice;
	m_nCurrentSlice = 0;
	if (direction>=0)
		rtval = distMin + half_dh;
	else
		rtval = distMax - half_dh;
	for (i=0; i<=nSlice; i++) m_listHead[i]=NULL;

	//=======build bucket=================
	_bucketSortElements(pelm, nelm, elmtype, direction, distMin, distMax);
	return rtval;
}

}
}