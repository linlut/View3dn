//tmeshobj.h

#ifndef __INC_QMESHOBJ_H__

#define __INC_QMESHOBJ_H__

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include "polyobj.h"


class CQuadObj: public CPolyObj
{

private:
	void _copyToObject(CQuadObj & aobj, const int objid);

	void _setData(Vector3d *v, const int nv, Vector4i *pBound, const int nt, const bool allocbuf=false)
	{
		copyVerticesAndConnection(v, nv, 4, (int *)pBound, nt, allocbuf);
	}

public:

	//Vector4i* m_pQuad; 

public:

	CQuadObj(void): CPolyObj()
	{
		//m_pQuad = NULL;
        m_nPlyVertexCount=4;
	}

	CQuadObj(Vector3d *pVertex, const int nVertex, 
		Vector4i *pBoundaryQuad, const int nQuad, const bool allocbuffer=false):
		CPolyObj()
	{
        m_nElmVertexCount=0;
        m_nPlyVertexCount=4;
		_setData(pVertex, nVertex, pBoundaryQuad, nQuad, allocbuffer);
		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

    CQuadObj(CQuadObj & a): CPolyObj()
	{
        m_nElmVertexCount = 0;
        m_nPlyVertexCount=4;
		_setData(a.m_pVertex, a.m_nVertexCount, (Vector4i*)a.m_pPolygon, a.m_nPolygonCount, true);
		AxisAlignedBox box;
		a.GetBoundingBox(box);
		this->SetBoundingBox(box);
	}

	virtual ~CQuadObj()
	{
        //SafeDeleteArray(m_pQuad);
		m_nPolygonCount = 0;
	}

	int SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[]);

	int exportPovrayFile(FILE *fp, const double *matrix=NULL);

	int saveSTLFile(const char *fname, const double* pmatrix=NULL);

    int saveTXTFile(const char *fname, const double* pmatrix=NULL);

	//=====================Drawing functions================================
    virtual double estimatedVertexRadius(void);

	virtual void glDrawElementFaceGoround(const int eid, void *pvoid);

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
		else if (ptype==2) n = m_nPolygonCount*4;	//edge
		return n;
	}

	virtual const char *Description(void) const
	{
		return "Quad";
	}

	virtual int PolygonCount(void)
	{
		return m_nPolygonCount;
	}

	virtual int TriangleCount(void)
	{
		return m_nPolygonCount*2;
	}

	virtual void getPolygonCoordinates(const int index, Vector3d v[], int &nv)
	{ 
		if (index<0 || index>m_nPolygonCount){
			nv = 0;
		}
        else{
			Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
		    const Vector4i & quad = m_pQuad[index];
		    v[0] = m_pVertex[quad.x];
		    v[1] = m_pVertex[quad.y];
		    v[2] = m_pVertex[quad.z];
		    v[3] = m_pVertex[quad.w];
		    nv = 4; 
        }
	}

	virtual void mirrorXYPlane(void)
	{
		//mirror the vertices
		CPolyObj::mirrorXYPlane();

		//reversion the boundary surface
		Vector4i * &m_pQuad = (Vector4i * &)m_pPolygon;
		for (int i=0; i<m_nPolygonCount; i++){
			int* pquad = &m_pQuad[i].x;
			const int _tmp = pquad[1];
			pquad[1] = pquad[3];
			pquad[3] = _tmp;
		}
	}


	//======================FILE IMPORT/EXPORT==============================
	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

	virtual int LoadFile(FILE *fp, const char *ftype);

	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL);

    virtual int exportElemConnectivity(FILE *fp, const int objid, const int tetbaseno, const int vbaseno);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double th=0);


};



void getRectangleSurface(const int nx, const int ny, Vector4i *&pQuad, int &nPolygonCount);


void doMesh2dRectangle(
        const Vector3d& lowleft, const Vector3d& upright, 
        const int nx, const int ny, const int nz,
		Vector3d *& pVertex, Vector4i *&pQuad, 
		int& nVertexCount, int& nPolygonCount);


void MergeQuadObj(CQuadObj & obja, CQuadObj & objb, CQuadObj &objc, const double errscale=2e-2);


#endif