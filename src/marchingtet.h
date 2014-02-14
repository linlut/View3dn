//Marchingtet.h

#ifndef __INC_MARCHINGTET_H__

#define __INC_MARCHINGTET_H__

#include <dynamic_array.h>
#include "tetraobj.h"
#include "vertexinfo.h"



class CMarchingTetEdge
{
public:
	int m_nP[2];					//vertex P0
	int m_nTime[2];					//time stamp0;
	bool m_bParticleFlag[2];		//particle flag for vertex P0;
	Vector3f m_SplitDelta[2];		//the cut point, with respect to P0
	float m_fT;						//the interpolation t

private:

	float _computeT(CVertexInfo *pVertInfo);


	float _getT(const int v, CVertexInfo *pVertInfo)
	{
		float t;
		ASSERT0(v==m_nP[0]||v==m_nP[1]);

		if (m_fT<0)
			m_fT=_computeT(pVertInfo);

		if (v==m_nP[0]) 
			t = m_fT;
		else
			t = 1-m_fT;
		return t;
	}

	void _splitVertex(const int tm, const int vid, const Vector3f& dvec, CVertexInfo *pVertInfo)
	{
		ASSERT0(vid==0 || vid==1);
		if (tm <= m_nTime[vid]) return;

		const int v = m_nP[vid];
		const float t = _getT(v, pVertInfo); 
		const bool b = pVertInfo[v].getParticleFlag();
		const bool r = b^m_bParticleFlag[vid];				
		if (b==false){  //v is still on the mesh, not converted to particle yet
			ASSERT0(r==false);
			m_SplitDelta[vid] = t * dvec;
		}
		else{
			if (r)
				m_SplitDelta[vid] = t * dvec;
		}
		m_nTime[vid] = tm;
		m_bParticleFlag[vid] = b;	
	}


public:

	CMarchingTetEdge(const int v0, const int v1)
	{
		if (v0<v1)
			m_nP[0] = v0, m_nP[1] = v1;
		else
			m_nP[0] = v1, m_nP[1] = v0;
		m_fT = -1;
		m_nTime[0] = m_nTime[1] = -1;
		m_bParticleFlag[0] = m_bParticleFlag[1] = false; 
		m_SplitDelta[0] = m_SplitDelta[1] = Vector3f(0,0,0);
	}

	//for the particle on the edge to find split position;
	Vector3f GetSplitPosition(const int v0, CVertexInfo *pVertInfo)
	{
		ASSERT0(m_nP[0]==v0 || m_nP[1]==v0);
		Vector3f pos = pVertInfo[v0].m_vCurrPosition;
		if (m_nP[0]==v0)
			pos += m_SplitDelta[0];
		else
			pos += m_SplitDelta[1];
		return pos;
	}

	Vector3f GetSplitTextCoor(CVertexInfo *pVertInfo)
	{
		const int v0=m_nP[0];
		const int v1=m_nP[1];
		ASSERT0(v0>=0 && v1>=0);
		const Vector3f& p0 = pVertInfo[v0].m_vInitPosition;
		const Vector3f& p1 = pVertInfo[v1].m_vInitPosition;
		const float t = _getT(v0, pVertInfo);
		if (t<-1e-10f)
			assert(0);
		const Vector3f tex = p0 + t*(p1-p0);
		return tex;		
	}

	//split the 
	void PerformSplit(const int tm, const int v0, const Vector3f &p0, const int v1, const Vector3f &p1, CVertexInfo *pVertInfo)
	{
		int i=0;
		ASSERT0((v0==m_nP[0] && v1==m_nP[1]) || (v1==m_nP[0] && v0==m_nP[1]));
		if (v0==m_nP[1]) i = 1;
		const Vector3f det = p1 - p0;
		_splitVertex(tm, i, det, pVertInfo);
	}

	float GetT(const int v, CVertexInfo *pVertInfo)
	{
		return _getT(v, pVertInfo);
	}

	bool AlreadyHasSplit(const int v)
	{
		int i = 0;
		ASSERT0(v==m_nP[0] || v==m_nP[1]);
		if (v==m_nP[1]) i = 1;
		const bool r= (m_nTime[i]>=0);
		return r;
	}

	void Lock(const int v, const int tm)
	{
		int i = 0;
		ASSERT0(v==m_nP[0] || v==m_nP[1]);
		if (v==m_nP[1]) i = 1;
		m_nTime[i] = tm;	
		m_bParticleFlag[i] = true;	
	}

	Vector3f GetSplitDelta(const int v)
	{
		ASSERT0(v==m_nP[0]||v==m_nP[1]);
		if (v==m_nP[0]) 
			return m_SplitDelta[0];
		else
			return m_SplitDelta[1];
	}

};


//Binary-search for marching tet edges;
inline int _lsearchTetEdgeArray(CDynamicArray<CMarchingTetEdge> *pSplitEdges, const int v0, const int v1)
{
	int x, y;
	const int nsize= pSplitEdges->GetSize();
	const CMarchingTetEdge *pEdges = pSplitEdges->GetBuffer();
	if (v0<v1)
		x = v0, y = v1;
	else
		x = v1, y = v0;

	for (int i=nsize-1; i>=0; i--){
		const CMarchingTetEdge *p = &pEdges[i];
		if ((x==p->m_nP[0]) && (y==p->m_nP[1]))
			return i;
	}
	return -1;		//search key not found
}


class CMarchingTet
{
public:
	Vector4i	m_tet;							//the four vertices;
	int			m_pMarchingTetEdge[6];			//indices to the marhcingtet edges;
	unsigned char m_nParticleExtractionOrder[4];//specify the order of tet split, which first, which second, etc.

private:

	int _getNoneParticleVertices(int vbuff[4], CVertexInfo *pVertInfo)
	{
		const int *a =  &m_tet.x;
		int n=0;
		for (int i=0; i<4; i++){
			const int v = a[i];
			if (!pVertInfo[v].getParticleFlag())
				vbuff[n++]=v;
		}
		return n;
	}

	int _vertexIndex(const int vid)
	{
		int pos=-1, *a = &m_tet.x;
		for (int i=0; i<4; i++){
			if (a[i]==vid){
				pos = i; 
				break;
			}
		}
		return pos;
	}

	//get tet edge id using vertex indices in the tet;
	int _tedgeid(const int v0, const int v1)
	{
		ASSERT0(v0>=0 && v0<4 && v1>=0 && v1<4);
		ASSERT0(v0!=v1);
		static unsigned char edgeid[4][4]={ 
			{-1, 0, 1, 2}, 
			{0, -1, 3, 4}, 
			{1, 3, -1, 5}, 
			{2, 4, 5, -1}};
		return (int)(edgeid[v0][v1]);
	}

	//get tet edge id using vertex global indices ;
	int _getEdgePointerByVertexIDs(const int v0, const int v1)
	{
		const int n0 = _vertexIndex(v0);
		const int n1 = _vertexIndex(v1);
		ASSERT0(n0!=-1 && n1!=-1);
		const int n2 = _tedgeid(n0, n1);
		return m_pMarchingTetEdge[n2];
	}

	int _getVerticeByExtractionSequence(const int order)
	{
		const int *a = &m_tet.x;
		for (int i=0; i<4; i++){
			if (m_nParticleExtractionOrder[i]==order)
				return a[i];
		}
		assert(0);
		return -1;
	}

	void _performSplit(const int tm, const int v0, const Vector3f &p0, const int v1, const Vector3f &p1, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo)
	{
		const int e = _getEdgePointerByVertexIDs(v0, v1);
		if (e<0) return;
		pSplitEdges[e].PerformSplit(tm, v0, p0, v1, p1, pVertInfo);
	}

	float _getT(const int v0, const int v1, CMarchingTetEdge* pSplitEdges, CVertexInfo *pVertInfo)
	{
		const int e = _getEdgePointerByVertexIDs(v0, v1);
		if (e<0) return 0.5f;
		const float t = pSplitEdges[e].GetT(v0, pVertInfo);
		return t;
	}

	void _reorderTetByExtractionSequence(const int order, int &v0, Vector3i & basetri, int edgeid[3])
	{
		ASSERT0(order>=0 && order<4);
		const int *a = &m_tet.x;
		for (int i=0; i<4; i++){
			if (m_nParticleExtractionOrder[i]==order){
				const int n1 = (i+1)%4;
				const int n2 = (i+2)%4;
				const int n3 = (i+3)%4;
				v0 = a[i];
				basetri.x = a[n1];
				basetri.y = a[n2];
				basetri.z = a[n3];

				edgeid[0] = _tedgeid(i, n1);
				edgeid[1] = _tedgeid(i, n2);
				edgeid[2] = _tedgeid(i, n3);
				return;
			}
		}
		//must find, so if not there is an error;
		assert(0);
	}

	void _appendOneEdge(const int e1, const int v0, const int v1, CDynamicArray<CMarchingTetEdge> *pSplitEdges)
	{
		if (m_pMarchingTetEdge[e1]>=0) return;
		int eindex = _lsearchTetEdgeArray(pSplitEdges, v0, v1);
		if (eindex<0){ 
			//add into the buffer;
			eindex = pSplitEdges->GetSize();
			CMarchingTetEdge edge(v0, v1);
			pSplitEdges->Add(edge);	
		}
		m_pMarchingTetEdge[e1] = eindex;
	}


	void _checkBoundaryTriangleOrientation(const Vector3i& tri, CVertexInfo *pVertInfo, Vector3i & newtri, int edgeid[3])
	{
		newtri = tri;
		const Vector3f center= (pVertInfo[m_tet.x].m_vInitPosition+
						  pVertInfo[m_tet.y].m_vInitPosition+
						  pVertInfo[m_tet.z].m_vInitPosition+
						  pVertInfo[m_tet.w].m_vInitPosition)*0.25f;
		const Vector3f &a = pVertInfo[tri.x].m_vInitPosition;
		const Vector3f &b = pVertInfo[tri.y].m_vInitPosition;
		const Vector3f &c = pVertInfo[tri.z].m_vInitPosition;
		const Vector3f ab = b - a;
		const Vector3f bc = c - b;
		const Vector3f P = center - a;
		const Vector3f N = CrossProd(ab, bc);
		const float r = DotProd(N, P);
		if (r<0){
			int tmp = newtri.x;
			newtri.x = newtri.y, newtri.y = tmp;	//swap
			tmp = edgeid[0], edgeid[0]=edgeid[1], edgeid[1] = tmp; //swap
		}
	}


	void _getVerticesAndEdgesByExtractionSequence(const int EXTRACT_ORDER, CVertexInfo *pVertInfo,
			int& v0, int& v1, int& v2, int& v3, 
			int& eidx0, int& eidx1, int& eidx2)
	{
		int _vid, edgeid[3];
		Vector3i basetri, basetri2;

		_reorderTetByExtractionSequence(EXTRACT_ORDER, _vid, basetri2, edgeid);
		_checkBoundaryTriangleOrientation(basetri2, pVertInfo, basetri, edgeid);
		v0 = _vid;
		v1 = basetri.x;
		v2 = basetri.y;
		v3 = basetri.z;
		const int e0= edgeid[0];
		const int e1= edgeid[1];
		const int e2= edgeid[2];
		ASSERT0((e0>=0 && e0<6) && (e1>=0 && e1<6) && (e2>=0 && e2<6));
		eidx0 = m_pMarchingTetEdge[e0];
		eidx1 = m_pMarchingTetEdge[e1];
		eidx2 = m_pMarchingTetEdge[e2];
		ASSERT0(eidx0!=-1 && eidx1!=-1 && eidx2!=-1);
	}

	void _getVerticesByExtractionSequence(const int EXTRACT_ORDER, CVertexInfo *pVertInfo, int& v0, int& v1, int& v2, int& v3)
	{
		int _vid, edgeid[3];
		Vector3i basetri, basetri2;
		_reorderTetByExtractionSequence(EXTRACT_ORDER, _vid, basetri2, edgeid);
		_checkBoundaryTriangleOrientation(basetri2, pVertInfo, basetri, edgeid);
		v0 = _vid;
		v1 = basetri.x;
		v2 = basetri.y;
		v3 = basetri.z;
	}

	void _genPatchForFirstCut(const int tm, 
		const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& p0, const Vector3f& p1, const Vector3f& p2, const Vector3f& p3, 
		CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);

	void _extrapolateVirtualTet4(
		 const int v0, const int v1, const int v2, const int v3, 
		 CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo, 
		 Vector3f &p1, Vector3f &p2, Vector3f &p3)
	{
		const int e1 = _getEdgePointerByVertexIDs(v0, v1);
		const int e2 = _getEdgePointerByVertexIDs(v0, v2);
		const int e3 = _getEdgePointerByVertexIDs(v0, v3);
		ASSERT0(e1>=0 && e2>=0 && e3>=0);
		const float t1 = pSplitEdges[e1].GetT(v0, pVertInfo);
		const float t2 = pSplitEdges[e2].GetT(v0, pVertInfo);
		const float t3 = pSplitEdges[e3].GetT(v0, pVertInfo);
		const Vector3f p01 = pSplitEdges[e1].GetSplitDelta(v0);
		const Vector3f p02 = pSplitEdges[e2].GetSplitDelta(v0);
		const Vector3f p03 = pSplitEdges[e3].GetSplitDelta(v0);
		p1 = p01*(1/t1);
		p2 = p02*(1/t2);
		p3 = p03*(1/t3);
		const Vector3f p0=pVertInfo[v0].m_vCurrPosition;
		p1+=p0; p2+=p0; p3+=p0;
	}

	void _interpolateTmpVertices4(
		 const int v0, const int v1, const int v2, const int v3, 
		 const Vector3f &p0, const Vector3f& p1, const Vector3f &p2, const Vector3f &p3, 
		 CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo, 
		 Vector3f &p01, Vector3f &p02, Vector3f &p03, 
		 Vector3f &p012, Vector3f &p013, Vector3f &p023, Vector3f &p0123)
	{
		const float K3=0.3333333f;
		const float K4=0.250f;
		const int e1 = _getEdgePointerByVertexIDs(v0, v1);
		const int e2 = _getEdgePointerByVertexIDs(v0, v2);
		const int e3 = _getEdgePointerByVertexIDs(v0, v3);
		if (e1<0)
			p01=(p0+p1)*0.5f;
		else
			p01 = pSplitEdges[e1].GetSplitDelta(v0)+p0;
		if (e2<0)
			p02=(p0+p2)*0.5f;
		else
			p02 = pSplitEdges[e2].GetSplitDelta(v0)+p0;
		if (e3<0)
			p03=(p0+p3)*0.5f;
		else
			p03 = pSplitEdges[e3].GetSplitDelta(v0)+p0;	
		p012 = (p0+p1+p2)*K3;
		p013 = (p0+p1+p3)*K3;
		p023 = (p0+p2+p3)*K3;
		p0123 = (p0+p1+p2+p3)*K4;
	}

	void _computeTexCoor4(
		const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo,
		Vector3f &t01, Vector3f &t02, Vector3f &t03, Vector3f &t012, Vector3f &t013, Vector3f &t023, Vector3f &t0123)
	{
		const float K3 = 0.3333333f;
		const float K4 = 0.2500f;
		const Vector3f t0 = pVertInfo[v0].m_vInitPosition;
		const Vector3f t1 = pVertInfo[v1].m_vInitPosition;
		const Vector3f t2 = pVertInfo[v2].m_vInitPosition;
		const Vector3f t3 = pVertInfo[v3].m_vInitPosition;

		const int e01=_getEdgePointerByVertexIDs(v0, v1);
		if (e01<0)
			t01=(t0+t1)*0.5f;
		else
			t01=pSE[e01].GetSplitTextCoor(pVertInfo);
		const int e02=_getEdgePointerByVertexIDs(v0, v2);
		if (e02<0)
			t02=(t0+t2)*0.5f;
		else
			t02=pSE[e02].GetSplitTextCoor(pVertInfo);
		const int e03=_getEdgePointerByVertexIDs(v0, v3);
		if (e03<0)
			t03=(t0+t3)*0.5f;
		else
			t03=pSE[e03].GetSplitTextCoor(pVertInfo);
		t012 = (t0+t1+t2)*K3;
		t013 = (t0+t1+t3)*K3;
		t023 = (t0+t2+t3)*K3;
		t0123=(t0+t1+t2+t3)*K4;
	}

	void _extrapolateVirtualTet2_1(
		 const int v0, const int v1, const int v2, const int v3, 
		 CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo, 
		 Vector3f &p01, Vector3f &p02, Vector3f &p03, 
		 Vector3f &p012, Vector3f &p013, Vector3f &p023, Vector3f &p0123)
	{
		const int e1 = _getEdgePointerByVertexIDs(v0, v1);
		const int e2 = _getEdgePointerByVertexIDs(v0, v2);
		const int e3 = _getEdgePointerByVertexIDs(v0, v3);
		const float t1 = pSplitEdges[e1].GetT(v0, pVertInfo);
		const float t2 = pSplitEdges[e2].GetT(v0, pVertInfo);
		const float t3 = pSplitEdges[e3].GetT(v0, pVertInfo);
		p01 = pSplitEdges[e1].GetSplitDelta(v0);
		p02 = pSplitEdges[e2].GetSplitDelta(v0);
		p03 = pSplitEdges[e3].GetSplitDelta(v0);

		const Vector3f pbase=pVertInfo[v0].m_vCurrPosition;
		const Vector3f p1 = p01*(1/t1);
		const Vector3f p2 = p02*(1/t2);
		const Vector3f p3 = p03*(1/t3);

		//======================================
		const int E12 = _getEdgePointerByVertexIDs(v1, v2);
		ASSERT0(E12!=-1);
		const float t12 = pSplitEdges[E12].GetT(v1, pVertInfo);
		const Vector3f p12 = p1+(p2-p1)*t12; 

		const int E13 = _getEdgePointerByVertexIDs(v1, v3);
		ASSERT0(E13!=-1);
		const float t13 = pSplitEdges[E13].GetT(v1, pVertInfo);
		const Vector3f p13 = p1+(p3-p1)*t13; 
		//======================================
		p012 = (p01+p12)*0.5f;
		p013 = (p01+p13)*0.5f;
		p023 = (p2+p3)*0.33333333f;
		p0123 = (p01+p12+p13)*0.33333333f;
		p01 += pbase; p02 += pbase;
		p03 += pbase; p012 += pbase;
		p013 += pbase; p023 += pbase;
		p0123 += pbase;
	}
	
	void _extrapolateVirtualTet2_2(
		 const int v0, const int v1, const int v2, const int v3, 
		 const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, 
		 CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo,
		 Vector3f &p02, Vector3f &p03, Vector3f &p12, Vector3f &p13, 
		 Vector3f &p012, Vector3f &p013, Vector3f &p023, Vector3f &p0123)
	{
		const float t01 =_getT(v0, v1, pSplitEdges, pVertInfo);
		const Vector3f p01 = p0 + t01*(p1-p0);
		const float t12 =_getT(v1, v2, pSplitEdges, pVertInfo);
		p12 = p1 + t12*(p2-p1);
		const float t13 =_getT(v1, v3, pSplitEdges, pVertInfo);
		p13 = p1 + t13*(p3-p1);
		p0123 = (p01+p12+p13)*0.33333333f;
		
		const float t02 =_getT(v0, v2, pSplitEdges, pVertInfo);
		p02 = p0 + t02*(p2-p0);
		const float t03 =_getT(v0, v3, pSplitEdges, pVertInfo);
		p03 = p0 + t03*(p3-p0);
		p023 = (p0+p2+p3)*0.33333333f;

		p012 = (p01+p12)*0.5f;
		p013 = (p01+p13)*0.5f;
	}

	void _lockEdges(const int tm, const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSplitEdges);

	void _extractSurface2_1(const int tm, 
		const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& P0, const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, 
		CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);

	void _computeTexCoor2_1(
		const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo,
		Vector3f &t01, Vector3f &t02, Vector3f &t03, Vector3f &t012, Vector3f &t013, Vector3f &t023, Vector3f &t0123);

	void _computeTexCoor2_2(
		const int v0, const int v1, const int v2, const int v3, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo,
		Vector3f &t02, Vector3f &t03, Vector3f &t12, Vector3f &t13, Vector3f &t012, Vector3f &t013, Vector3f &t023, Vector3f &t0123);

	void _extractSurface1(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);
	void _extractSurface2(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);
	void _extractSurface3(const int tm, CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);
	void _extractSurface4(const int tm, const int v0, const int v1, const int v2, const int v3, 
		const Vector3f& P0, const Vector3f& P1, const Vector3f& P2, const Vector3f& P3, const bool useP,
		CMarchingTetEdge *pSplitEdges, CVertexInfo *pVertInfo);

	void _decideVertexFlags(const int v0, const int v1, const int v2, const int v3, 
			const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, const Vector3f &p3, 
			CVertexInfo *pVertInfo, int f[11], Vector3f norms[11]);

	void _getMidEdgePointAttr(const int v0, const int v1, 
			const Vector3f& p0, const Vector3f& p1, CVertexInfo *pVertInfo, int &f, Vector3f &norm);

	void _getMidTrianglePointAttr(const int v0, const int v1, const int v2, 
			const Vector3f &p0, const Vector3f &p1, const Vector3f &p2, 
			CVertexInfo *pVertInfo, int &f, Vector3f &norm);

public:

	CMarchingTet(const Vector4i& tet)
	{
		m_tet = tet;
		m_pMarchingTetEdge[0]=m_pMarchingTetEdge[1]=
		m_pMarchingTetEdge[2]=m_pMarchingTetEdge[3]=
		m_pMarchingTetEdge[4]=m_pMarchingTetEdge[5]=-1;

		m_nParticleExtractionOrder[0]=m_nParticleExtractionOrder[1]=
		m_nParticleExtractionOrder[2]=m_nParticleExtractionOrder[3]=0xff;
	}
	
	int MaterialOfTet(CVertexInfo *pVertInfo)
	{
		int m0 = pVertInfo[m_tet.x].getMaterialID();
		int m1 = pVertInfo[m_tet.y].getMaterialID();
		int m2 = pVertInfo[m_tet.z].getMaterialID();
		int m3 = pVertInfo[m_tet.w].getMaterialID();
		if (m0==m1 && m1==m2 && m2==m3)
			return m0;
		ASSERT0(0);
		return -1;
	}

	int GetSplitVertexCount(void)
	{
		int i, c;
		for (i=c=0; i<4; i++){
			unsigned char s = m_nParticleExtractionOrder[i];
			if (s!=0xff) c++;
		}
		return c;
	}

	int GetNonSplitVertices(int vbuff[4])
	{
		int i, c;
		const int *a = &m_tet.x;
		for (i=c=0; i<4; i++){
			unsigned char s = m_nParticleExtractionOrder[i];
			if (s==0xff)
				vbuff[c++] = a[i];
		}
		return c;
	}


	void AddOneSplitVertex(const int vid)
	{
		const int insertpos=_vertexIndex(vid);
		ASSERT0(insertpos>=0 && insertpos<4);
		if (m_nParticleExtractionOrder[insertpos]!=0xff)  	//If already inserted, 
			return;											//do nothing
		const int n = GetSplitVertexCount();
		m_nParticleExtractionOrder[insertpos]=n;
	}

	void AddThreeSplitEdgesForVertex(const int vid0, CDynamicArray<CMarchingTetEdge> *pSplitEdges)
	{
		const int v0 = _vertexIndex(vid0);
		const int v1 = (v0+1)%4;
		const int v2 = (v0+2)%4;
		const int v3 = (v0+3)%4;
		const int *a = &(m_tet.x);
		const int vid1 = a[v1];
		const int vid2 = a[v2];
		const int vid3 = a[v3];

		const int eid1 = _tedgeid(v0, v1);
		_appendOneEdge(eid1, vid0, vid1, pSplitEdges);

		const int eid2 = _tedgeid(v0, v2);
		_appendOneEdge(eid2, vid0, vid2, pSplitEdges);

		const int eid3 = _tedgeid(v0, v3);
		_appendOneEdge(eid3, vid0, vid3, pSplitEdges);
	}

	void LockVertexAttachedEdges(const int tm, const int v0,CMarchingTetEdge *pSplitEdges)
	{
		int v1, v2, v3;
		if (v0==m_tet.x)
			v1=m_tet.y, v2=m_tet.z, v3=m_tet.w;
		else if (v0==m_tet.y)
			v1=m_tet.z, v2=m_tet.w, v3=m_tet.x;
		else if (v0==m_tet.z)
			v1=m_tet.w, v2=m_tet.x, v3=m_tet.y;
		else 
			v1=m_tet.x, v2=m_tet.y, v3=m_tet.z;
		_lockEdges(tm, v0, v1, v2, v3, pSplitEdges);
	}


	int GetVerticeByExtractionSequence(const int order)
	{
		return _getVerticeByExtractionSequence(order);
	}


	void PreSlitAllEdges(const int tm, CMarchingTetEdge* pSplitEdges, CVertexInfo *pVertInfo);


	void ExtractSurface(const int tm, CMarchingTetEdge* pSE, CVertexInfo *pVertInfo);

	void _ExtractSurface(const int tm, CMarchingTetEdge *pSE, CVertexInfo *pVertInfo);
					   

};


#endif