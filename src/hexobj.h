//tmeshobj.h

#ifndef __INC_HEXOBJ1246_H__

#define __INC_HEXOBJ1246_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include "qmeshobj.h"
#include "geomath.h"


class CHexObj: public CQuadObj
{

private:
	void _drawSolidElementLine(void);

	void _setData(Vector3d *pVertex, const int nVertex, Vector8i *pElement, const int nElement, 
				  Vector4i *pBoundaryQuad, const int nQuad, const bool allocbuffer=false);

public:
	

public:

	CHexObj(void): CQuadObj()
	{
        m_nElmVertexCount = 8;
	}

	
	CHexObj(Vector3d *pVertex, const int nVertex, Vector8i *pElement, const int nElement, 
		Vector4i *pBoundaryQuad, const int nQuad, const bool allocbuffer=false):
		CQuadObj()
	{
        m_nElmVertexCount = 8;
		_setData(pVertex, nVertex, pElement, nElement, pBoundaryQuad, nQuad, allocbuffer);
		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

    CHexObj(CHexObj & a): CQuadObj()
	{
        m_nElmVertexCount = 8;
		_setData(a.m_pVertex, a.m_nVertexCount, 
			     (Vector8i*)a.m_pElement, a.m_nElementCount, 
				 (Vector4i*)a.m_pPolygon, a.m_nPolygonCount, true);
		AxisAlignedBox box;
		a.GetBoundingBox(box);
		this->SetBoundingBox(box);
	}


	virtual ~CHexObj()
	{
        m_nElementCount=0;
	}


	virtual void mirrorXYPlane(void);


	void exportNeutralMeshFile(FILE *fp, const double *matrix=NULL);

	//return the array of boundary vertices and count
	int *getBoundaryVeritces(int &nBoundVertA);


	//=============draw func==============================================
	virtual void glDraw(const CGLDrawParms &dparms);

	virtual double estimatedVertexRadius(void);

	//virtual void drawVolumetricElements(void);

	//======================Picking functions===============================
	virtual const char *Description(void) const
	{
		return "Hexobj";
	}


	//====================Export to various formats;====================

	virtual int LoadFile(const char *fname);

	virtual int exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL)
    {
        const int r = CPolyObj::exportElemNodes(fp, baseno, matrix);
        return r;
    }

	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL);

	virtual void propergateFemBoundSeed(const int tid, const float th)
	{
		CPolyObj::propergateFemBoundSeed(tid, th, m_pPolygon, 4);
	}

	virtual double* computeElementVolumes(void);

	virtual double* computeVertexVolumes(void);


};



//Generate hex meshes for a brick 
void doMeshBrick(const Vector3d& lowleft, const Vector3d& upright, const int nx, const int ny, const int nz, 
		Vector3d *& m_pVertex, Vector4i *&m_pQuad, Vector8i *&m_pHex, 
		int& m_nVertexCount, int& m_nPolygonCount, int& m_nHexCount);

void getBrickElements(const int nx, const int ny, const int nz, Vector8i *&m_pHex, int& m_nHexCount);

void getBrickBounday(const int nx, const int ny, const int nz, Vector4i *&m_pQuad, int& m_nPolygonCount);

void MergeHexObj(CHexObj & obja, CHexObj & objb, CHexObj &objc, const double tolscale=1E-2);

#endif