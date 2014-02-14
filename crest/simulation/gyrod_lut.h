//FILE: gyrod_lut.h
#ifndef _INC_GYROD_LUT_NZ2009_H_
#define _INC_GYROD_LUT_NZ2009_H_

#include <math.h>
#include <vector>
#include <vectorall.h>
#include <matrix3x3.h>

#define NORMAL_SCALE 1000

//-----------------------------------------------------------------------------
//    FUNCTION asmFastInvSqrt (BYREF x AS SINGLE) AS SINGLE
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------
//  Note: This Function is a PB-ASM port of C-Code originally found at:
//  http://www.geometrictools.com/Docume...nverseSqrt.pdf
inline float InvSqrt(const float _x)
{
	float x = _x;
	const float xhalf = 0.5f*x;
	int i = *((int*)&x);
	i = 0x5f3759df - (i >> 1); // This line hides a LOT of math!
	x = *((float*)&i);
	x = x*(1.5f - xhalf*x*x);  // First-Pass
	x = x*(1.5f - xhalf*x*x);  // Repeated for better approximation
	return x;
}

inline int DotProd(const Vector3i &a, const Vector3s &b){ return a.x*b.x+a.y*b.y+a.z*b.z; }

class AnchorVertexInfo
{
public:
	Vector3s m_dir_short;	//quatized direction vector, scaled with a constant 
	unsigned char m_count;
	unsigned char m_level;
	Vector3d m_dir;			//normalized direction vector
	double3x3 m_F;			//deformation gradient 

public:
	AnchorVertexInfo(void){}

	//dir should be normalized
	AnchorVertexInfo(const Vector3d& dir, const double3x3& F)
	{
		m_dir = dir; 
		Vector3d t = dir * NORMAL_SCALE;
		m_dir_short = Vector3s((short)t.x, (short)t.y, (short)t.z);
		m_count = m_level = 0;
		m_F = F;
	}
};

class DGCubeMap
{
private:
	const int IMAGEWIDTH;
	const double W05;
	const double W05W105;
	unsigned int *m_pImage[6];		//the order is +X, -X plane, 
									//			   +Y, -Y
									//			   +Z, -Z
private:
	int _getCloestAnchorVertex(const Vector3d &pos, const AnchorVertexInfo *pVertex, const int NU, const int NV) const;

	int _getCloestAnchorVertex(const Vector3d &pos, const AnchorVertexInfo *pVertex, const int nv) const;

	inline Vector3d _getWorldPos(const int i, const int j, const int planeID)
	{
		ASSERT0(planeID>=0 && planeID<6);
		Vector3d p;
		int ix, iy, iz;
		const int planeID2 = planeID>>1;
		switch(planeID2){
			case 0: ix=1, iy=2, iz=0; break;
			case 1: ix=2, iy=0, iz=1; break;
			case 2: ix=0, iy=1, iz=2; break;
		}
		const double PIXELSIZE = 2.0/(IMAGEWIDTH-1);
		const double K = 0.5*(1-IMAGEWIDTH);
		const double fx = (i + K)*PIXELSIZE;
		const double fy = (j + K)*PIXELSIZE;
		p[ix]=fx; p[iy]=fy; p[iz]=1;
		if (planeID&0x1) p[iz]=-1;
		return p;
	}

public:
	DGCubeMap(void);

	~DGCubeMap(void);

	//direction vector dir needs not to be normalized, but must be none zero
	inline int getCubemapPixelValue(const Vector3d &dir) const
	{
		//decide the plane 
		int ix=1, iy=2, iz=0;
		double maxval = fabs(dir.x);
		const double fabsy = fabs(dir.y);
		const double fabsz = fabs(dir.z);
		if (maxval<fabsy) maxval=fabsy, ix=2, iy=0, iz=1;
		if (maxval<fabsz) maxval=fabsz, ix=0, iy=1, iz=2;
		int iz2 = iz<<1;
		if (dir[iz]<0) iz2++;
		const unsigned int *pImage = m_pImage[iz2];
		//find the pixel location
		const double finv = W05/maxval;
		int iposx = (int)(dir[ix]*finv + W05W105);
		int iposy = (int)(dir[iy]*finv + W05W105);
		/*
		if (iposx<0 || iposx>=IMAGEWIDTH){ 
			printf("Address error!\n");
			iposx = IMAGEWIDTH>>1;
		}
		if (iposy<0 || iposy>=IMAGEWIDTH){
			printf("Address error!\n");
			iposy = IMAGEWIDTH>>1;
		}
		*/
		//printf("IZ2=%d u=%d v=%d\n", iz2, iposx, iposy);
		return pImage[iposy*IMAGEWIDTH+iposx];
	}

	void initCubeMap(const AnchorVertexInfo *pVertex, const int NU, const int NV);

	void testFunc(void);
};

//==================================================================
//inline int DotProd(const Vector3i &a, const Vector3s &b){ return a.x*b.x+a.y*b.y+a.z*b.z; }
//inline Vector3d midpoint_slerp(const Vector3d& a, const Vector3d &b){Vector3d n = (a+b)*0.5; n.Normalize(); return n;}

class SphericalQuadTree
{
private:
	class LeafNode
	{
	public:
		Vector3s m_tri;		//IDs for vertices;
		
		LeafNode(void){}

		LeafNode(const int i0, const int i1, const int i2)
		{
			m_tri = Vector3s(i0, i1, i2);
		}
	};

	class NoneLeafNode
	{
	public:
		Vector3s m_childNorm[4];
		short int m_childID[4];

	public:
		NoneLeafNode(void){}

		NoneLeafNode(const Vector3d norm[4], const short childID[4])
		{
			for (int i=0; i<4; i++){
				Vector3d n = norm[i]*NORMAL_SCALE;
				m_childNorm[i] = Vector3s((short)n.x, (short)n.y, (short)n.z);
			}
			m_childID[0]=childID[0];
			m_childID[1]=childID[1];
			m_childID[2]=childID[2];
			m_childID[3]=childID[3];
		}
	};

private:
	const int NORMAL_DOTPROD_TOL;	//about 5.625 degree from the north pole
	int m_nMaxLevel;
	vector<AnchorVertexInfo> m_vertices;
	vector<NoneLeafNode> m_nodes;
	vector<LeafNode> m_leaves;
	int XPLUSid, YPLUSid, ZPLUSid;
	int XMINid, YMINid, ZMINid;

	inline int _getVertexIndex(const Vector3d& v)
	{
		const double ZTOL = 1e-12;
		const unsigned int nsize = m_vertices.size();
		for (unsigned int i=6; i<nsize; i++){
			const double d2 = Distance2(v, m_vertices[i].m_dir);
			if (d2<ZTOL) return i;
		}
		const int ipos = m_vertices.size();
		double3x3 F; F.setZeroMatrix();
		AnchorVertexInfo p(v, F);
		m_vertices.push_back(p);
		return ipos;
	}

	void _buildTree(const int nodeid, 
		const Vector3d &p0, const int i0, const Vector3d &p1, const int i1, const Vector3d &p2, const int i2, 
		const int level, const int maxlevel);

	inline int _searchLevel0(const Vector3i& dir)
	{
		const int x = dir.x>=0?1:0;
		const int y = dir.y>=0?1:0;
		const int z = dir.z>=0?1:0;
		return x|(y<<1)|(z<<2);
	}

	inline int _searchNoneLeafOnce(const int nodeid, const Vector3i& dir) const 
	{
		//compute dotprod for all 4 subtri.
		const NoneLeafNode *p = &m_nodes[nodeid];
		int dotmax = DotProd(dir, p->m_childNorm[0]);
		int childnodeid = p->m_childID[0];
		const int dotp1 = DotProd(dir, p->m_childNorm[1]);
		const int dotp2 = DotProd(dir, p->m_childNorm[2]);
		const int dotp3 = DotProd(dir, p->m_childNorm[3]);
		//find the closet one
		if (dotmax<dotp1) dotmax=dotp1, childnodeid=p->m_childID[1];
		if (dotmax<dotp2) dotmax=dotp2, childnodeid=p->m_childID[2];
		if (dotmax<dotp3) dotmax=dotp3, childnodeid=p->m_childID[3];
		return childnodeid;
	}

	inline AnchorVertexInfo* _searchLeafOnce(const int leafnodeID, const Vector3i& dir)
	{
		const Vector3s& t = m_leaves[leafnodeID].m_tri;
		AnchorVertexInfo *p0 = &m_vertices[t.x];
		int dotp0 = DotProd(dir, p0->m_dir_short);
		AnchorVertexInfo *p1 = &m_vertices[t.y];
		int dotp1 = DotProd(dir, p1->m_dir_short);
		AnchorVertexInfo *p2 = &m_vertices[t.z];
		int dotp2 = DotProd(dir, p2->m_dir_short);
		if (dotp1>dotp0) p0=p1, dotp0=dotp1;
		if (dotp2>dotp0) p0=p2;
		return p0;
	}

	void _propergateDeformationTensor(const int passid, const int i, const int j);

	void _initDeformationTensors(void);

public:
	SphericalQuadTree(const int ilevel);

	~SphericalQuadTree()
	{
		m_vertices.clear();
		m_nodes.clear();
		m_leaves.clear();
	}

	inline AnchorVertexInfo *findLookupTableElement(const Vector3d& dir)
	{
		const double ZERO00 = 1e-20;
		Vector3d dirf(dir.x, dir.y, dir.z);
		const double len2 = Magnitude2(dirf)+ZERO00;
		const double invlen = InvSqrt(len2);
		const double invlen_scl = invlen * NORMAL_SCALE;

		//quantize the direction and exclude still cases
		const int dx = (int)(dirf.x*invlen_scl);
		const int dy = (int)(dirf.y*invlen_scl);
		const int dz = (int)(dirf.z*invlen_scl);
		const Vector3i diri(dx, dy, dz);
		if (diri.z>=NORMAL_DOTPROD_TOL){
			return &m_vertices[0];
		}
		ASSERT0(m_nMaxLevel==3);
		const int i0 = _searchLevel0(diri); 
		const int i1 = _searchNoneLeafOnce(i0, diri);
		const int i2 = _searchNoneLeafOnce(i1, diri);
		const int i3 = _searchNoneLeafOnce(i2, diri);
		AnchorVertexInfo* p = _searchLeafOnce(i3, diri);
		return p;
	}

	inline static SphericalQuadTree *getInstance(void)
	{
		static SphericalQuadTree* pInst = NULL;
		if (pInst == NULL)
			pInst = new SphericalQuadTree(3);
		return pInst;
	}

	void exportMesh(const char *fname);
};

//==============================================================

typedef AnchorVertexInfo CLookupTableElement;

class CGyrodLookupTable
{
private:
	int m_nu;		//number of points in the u direction	
	int m_nv;		//number of points in the v dir
	double m_Scale;	//scaling factor = (m_nv-1)/PI
	CLookupTableElement *m_pElement;	
	DGCubeMap *m_pCubeMap;

	inline const int _getLookupTableElementIndex(const int u, const int v) const
	{
		ASSERT0(u>=0 && u<m_nu); 
		ASSERT0(v>=0 && v<m_nv);
		return (v * m_nu + u);
	}

	inline const CLookupTableElement* _getLookupTableElement(const int u, const int v) const
	{
		const int i = _getLookupTableElementIndex(u, v);
		return &(m_pElement[i]);
	}

	void _genLookupTable(void);

public:

	CGyrodLookupTable(const int ndiv=16);

	~CGyrodLookupTable(void);

	inline const CLookupTableElement* findLookupTableElement(const Vector3f &dir) const
	{
		int ix, iy;
		//first, compute beta
		const float len2 = Magnitude2(dir);
		//double rodlen = sqrt(len2);
		const float r1 = InvSqrt(len2);
		//const float r1 = (float)(1.0/rodlen);
		const float z = dir.z*r1;
		if (z>0.995f){
			iy=0; ix = 0;
		}
		else{
			const float beta = acos(z);
			iy = (int)(beta * m_Scale + 0.5f);
			//2nd, compute alpha
			const float alpha = atan2(dir.y, dir.x);
			ix = (int)(alpha * m_Scale + 0.5f); 
			if (ix<0) ix+=(m_nu-1);
		}
		//return values
		const CLookupTableElement* r = this->_getLookupTableElement(ix, iy);
		return r;
	}

	inline const CLookupTableElement* findLookupTableElement(const Vector3d &dir) const
	{
		const Vector3f p((float)dir.x, (float)dir.y, (float)dir.z);
		return findLookupTableElement(p);
	}

	inline const CLookupTableElement* findLookupTableElementCubeMap(const Vector3d &dir) const
	{
		ASSERT0(m_pCubeMap!=NULL);
		const int ipos = m_pCubeMap->getCubemapPixelValue(dir);
		return &m_pElement[ipos];
	}
};

#endif