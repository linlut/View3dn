//tmeshobj.h

#ifndef __INC_LINEOBJ_31H__
#define __INC_LINEOBJ_31H__

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include "polyobj.h"
#include "geomath.h"


class CLineObj: public CPolyObj
{

private:
	void _setData(Vector3d *v, const int nv, Vector2i *pBound, const int nt, const bool allocbuf=false)
	{
		copyVerticesAndConnection(v, nv, 2, (int *)pBound, nt, allocbuf);
	}

	void _copyToObject(CLineObj & aobj, const int objid=-1);

public:

public:

	CLineObj(Vector3d *v, const int nv, Vector2i *pLine, const int nl, const bool allocbuf=false):
		CPolyObj()
	{
        m_nPlyVertexCount=2;
        m_nElmVertexCount=0;
		_setData(v, nv, pLine, nl, allocbuf);
		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

	CLineObj(void): CPolyObj()
	{
        m_nPlyVertexCount = 2;
        m_nElmVertexCount = 0;
	}

	virtual ~CLineObj()
	{
	}

	//===================FILE IO============================================
	virtual int SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[]);

	int exportPovrayFile(FILE *fp, const double *matrix=NULL);

	//=====================Drawing functions================================
	
	virtual void glDraw(const CGLDrawParms &dparms);

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
		else if (ptype==2) n = m_nPolygonCount*2;	//edge
		return n;
	}

	virtual const char *Description(void) const
	{
		return "line";
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
	virtual int exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double th=0);

	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

	//==========================================================

	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL);

};









#endif