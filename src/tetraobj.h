//tmeshobj.h

#ifndef __INC_TETRAOBJ_H__

#define __INC_TETRAOBJ_H__

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>
#include "tmeshobj.h"
#include "geomath.h"


class CTetraObj: public CTriangleObj
{

private:
	void OptimizeVertices(Vector3d* pVertex, const double& th, int *pIDBuffer);

	void OptimizeConnectivity(int *pIDBuffer);

	void _drawSolidElementLine(void);

	void _setData(Vector3d *pVertex, const int nVertex, Vector4i *pElement, const int nElement, 
				  Vector3i *pBoundaryTri, const int nTri, const bool allocbuffer=false);

public:
	//Vector4i* m_pTetra; 

public:

	CTetraObj(void): CTriangleObj()
	{
        m_nElmVertexCount = 4;
	}


	CTetraObj(Vector3d *pVertex, const int nVertex, 
		Vector4i *pElement, const int nElement, 
		Vector3i *pTri, const int nTri, const bool allocbuffer=false);


	virtual ~CTetraObj()
	{
	}

	void exportNeutralMeshFile(FILE *fp, const double *matrix=NULL);

	//=============draw func==============================================
	virtual void glDraw(const CGLDrawParms &dparms);

	virtual void drawVolumetricElements(void);

	//======================Picking functions===============================
	virtual const char *Description(void) const
	{
		return "Tetra";
	}

	void OptimizeMesh(Vector3d* pVertex, const double& th);


	//====================Export to various formats;====================

	virtual int LoadFile(const char *fname);

	virtual int exportElemConnectivity(FILE *fp, const int objid, const int objsetid, const int tetbaseno, const int vbaseno, const int *vbasebuffer);

	virtual int exportElemNodes(FILE *fp, const int baseno=0, const double *matrix=NULL)
    {
        const int r = CPolyObj::exportElemNodes(fp, baseno, matrix);
        return r;
    }

	virtual void exportFile(FILE *fp, const char *format, const double *matrix=NULL);

	virtual void propergateFemBoundSeed(const int tid, const float THRESHOLD)
	{
		CPolyObj::propergateFemBoundSeed(tid, THRESHOLD, m_pPolygon, 3);
	}

	virtual double* CTetraObj::computeElementVolumes(void);

	virtual double* CTetraObj::computeVertexVolumes(void);

};






#endif