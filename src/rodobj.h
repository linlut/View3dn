//FILE: rodobj.h

#ifndef __INC_RODOBJ_NZ20080226H__
#define __INC_RODOBJ_NZ20080226H__

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include "polyobj.h"
#include "geomath.h"


class CRodObj: public CPolyObj
{

private:
	void _setData(Vector3d *v, const int nv, Vector2i *prod, const int nt, const bool allocbuf=false)
	{
		PolyCopyVertices(nv, v, allocbuf, m_nVertexCount, m_pVertex);
		PolyCopyElements(nt, (int*)prod, 2,  allocbuf, m_nElementCount, m_pElement);
	}

	void _copyToObject(CRodObj & aobj, const int objid=-1);

public:

	CRodObj(Vector3d *v, const int nv, Vector2i *pRod, const int nrod, const bool allocbuf=false):
		CPolyObj()
	{
        m_nPlyVertexCount=0;
        m_nElmVertexCount=2;
		_setData(v, nv, pRod, nrod, allocbuf);
		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

	CRodObj(void): CPolyObj()
	{
        m_nPlyVertexCount = 0;
        m_nElmVertexCount = 2;
	}

	virtual ~CRodObj()
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
		else if (ptype==1) n = m_nElementCount;		//face
		else if (ptype==2) n = m_nElementCount*2;	//edge
		return n;
	}

	virtual const char *Description(void)
	{
		return "rod";
	}

	virtual void getPolygonCoordinates(const int index, Vector3d v[], int &nv)
	{ 
		nv = 0;
		return;
	}

	virtual double estimatedVertexRadius(void);


	//====================Export to various formats;====================
	virtual int exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL, const double th=0);

	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

};









#endif