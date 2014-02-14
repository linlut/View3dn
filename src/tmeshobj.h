//tmeshobj.h

#ifndef __INC_TMESHOBJ_H__

#define __INC_TMESHOBJ_H__

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include "polyobj.h"
#include "geomath.h"


class CTriangleObj: public CPolyObj
{

private:
	void _copyToObject(CTriangleObj & aobj, const int objid=-1);

	void _setData(Vector3d *v, const int nv, Vector3i *pBound, const int nt, const bool allocbuf=false)
	{
		copyVerticesAndConnection(v, nv, 3, (int *)pBound, nt, allocbuf);
	}

	Vector3f ComputeTriangleNormal(const int index, const bool areaweighted=false) const
	{
		Vector3i *m_pTriangle = (Vector3i*)m_pPolygon;
		ASSERT0(index>=0 && index<m_nPolygonCount);
		Vector3i &tri=m_pTriangle[index];
		const Vector3d & a = m_pVertex[tri[0]];
		const Vector3d & b = m_pVertex[tri[1]];
		const Vector3d & c = m_pVertex[tri[2]];
		Vector3d n;
		if (areaweighted){
			Vector3d e0 = b - a;
			Vector3d e1 = c - a;
			n = CrossProd(e0, e1);
		}
		else
			n=compute_triangle_normal(a, b, c);
        Vector3f nf (n.x, n.y, n.z);
        return nf;
	}

	int _loadTXTFile(FILE *fp);

	int _loadSTLFile(FILE *fp);

	int _loadPLYFile(FILE *fp);

public:

	//Vector3i* m_pTriangle; 

public:

	CTriangleObj(Vector3d *pVertex, const int nVertex, 
		Vector3i *pBoundaryTri, const int nTri, const bool allocbuffer=false):
		CPolyObj()
	{
        m_nElmVertexCount=0;
        m_nPlyVertexCount=3;
		_setData(pVertex, nVertex, pBoundaryTri, nTri, allocbuffer);
		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

	CTriangleObj(void): CPolyObj()
	{
		//m_pTriangle = NULL;
        m_nPlyVertexCount = 3;
	}

	virtual ~CTriangleObj()
	{
		//if (m_pTriangle)
		//	delete [] m_pTriangle;
		//m_pTriangle = NULL;
	}

	virtual int SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[]);

	//=====================Drawing functions================================
	virtual void glDrawElementFaceGoround(const int eid, void* mirrorred=NULL);

	virtual void glDrawElementFacePhong(const int eid, void *pvoid);

	virtual void glDrawElementLine(const int eid, const float offset, const bool needtex);

	virtual void ComputePolygonNormals(void);

    virtual void ComputeVertexNormals(void);

    virtual void flipNormal(void);

	//======================Picking functions===============================
	virtual void DrawPickingObject(const int objid);

	virtual void DrawPickingObjectFace(const int objid);

	virtual void DrawPickingObjectLine(const int objid);

	virtual void GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1);

	virtual void GetPickedLine(const int eid, int & v0, int &v1);

	virtual void GetPickedPlane(const int eid, Vector3d v[4]);

	virtual int GetPickableElementCount(const int ptype)
	{
		int n=0;
		if (ptype==0) n = 1;						//obj
		else if (ptype==1) n = m_nPolygonCount;		//face
		else if (ptype==2) n = m_nPolygonCount*3;	//edge
		return n;
	}


	virtual const char *Description(void) const
	{
		return "Tri";
	}


	virtual int PolygonCount(void)
	{
		return m_nPolygonCount;
	}

	virtual int TriangleCount(void)
	{
		return m_nPolygonCount;
	}

	virtual void getPolygonCoordinates(const int index, Vector3d v[], int &nv)
	{ 
		if (index<0 || index>m_nPolygonCount){
			nv = 0;
			return;
		}
		Vector3i* m_pTriangle = (Vector3i*)m_pPolygon;
		assert(m_pPolygon!=NULL);
		const Vector3i & quad = m_pTriangle[index];
		v[0] = m_pVertex[quad.x];
		v[1] = m_pVertex[quad.y];
		v[2] = m_pVertex[quad.z];
		nv = 3; 
	}

	//====================Export to various formats;====================
	virtual int exportElemConnectivity(FILE *fp, const int objid, const int tetbaseno, const int vbaseno);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double th=0);

	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

	virtual int LoadFile(FILE *fp, const char *ftype);

	virtual int LoadFile(const char *fname);

	int exportPovrayFile(FILE *fp, const double *matrix=NULL);

	int saveSTLFile(const char *fname, const double* pmatrix=NULL);

    int saveTXTFile(const char *fname, const double* pmatrix=NULL);

	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL);

	virtual bool saveMeshWavefrontObj(const char *fname, const double *matrix=NULL);

};

#endif