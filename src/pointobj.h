//pointobj.h
// the class has been extended to draw not only spheres, but also ellipsoids
// Nan Zhang, Feb. 27, 2011

#ifndef __INC_CIAPOINTOBJ_H__

#define __INC_CIAPOINTOBJ_H__

#if defined(_WIN32)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "polyobj.h"
#include "matrix3x3.h"


class CPointObj: public CPolyObj
{
private:
	void _copyToObject(CPointObj & aobj, const int objid);

	void _exportPovrayFile(FILE *fp, const double *matrix=NULL);

public:
	float* m_pRadius;
	float3x3* m_pMatrix;//let ellipsoid vertex be y, unit sphere vertex be x
						// y = VD^(-0.5) * x, where V is the rotation matrix, D is 
						// a diagonal matrix of the axis lengths (eigen value / vector)

public:
	CPointObj(void): CPolyObj()
	{
        m_nPlyVertexCount = 1;
		m_pRadius = NULL;
		m_pMatrix = NULL;
	}

	CPointObj(Vector3d *pVertex, const int nVertex, const float r, const bool allocbuffer=false):
		CPolyObj()
	{
        m_nPlyVertexCount = 1;
		m_pRadius = NULL;
		m_pMatrix = NULL;
		m_nVertexCount = nVertex;
		m_fGivenRad = r;

		if (allocbuffer){
			m_pVertex = new Vector3d [nVertex];
			assert(m_pVertex!=NULL);
			for (int i=0; i<nVertex; i++) m_pVertex[i]=pVertex[i];
		}
		else
			m_pVertex = pVertex;

		AxisAlignedBox box;
		ComputeBoundingBox(box.minp, box.maxp);
		SetBoundingBox(box);
	}

	virtual ~CPointObj()
	{
		delete [] m_pRadius; m_pRadius = NULL;
		delete [] m_pMatrix; m_pMatrix = NULL;
	}

	int LoadPltFileWithoutHeader(FILE *fp, const int nv, const int nf, const int nattrib);

	virtual int LoadFile(FILE *fp, const char *ftype);

	virtual int SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[]);

	virtual void ComputeBoundingBox(Vector3d& lowleft, Vector3d &upright)
	{
		CPolyObj::ComputeBoundingBox(lowleft, upright);
		Vector3d disp(m_fGivenRad, m_fGivenRad, m_fGivenRad);
		lowleft -= disp;
		upright += disp;
	}

	//for point obj, point normal is not required at this time!!
	virtual void checkNormals(const bool recompute){}

	//=====================Drawing functions================================

	virtual void glDrawElementFaceGoround(const int eid, void *pvoid);

	virtual void glDrawElementFacePhong(const int eid, void *pvoid);

	virtual void glDrawElementLine(const int eid, const float offset, const bool needtex);

	virtual void glDrawVertexSphere(const int eid, void *p);

	//======================Picking functions===============================
	virtual void DrawPickingObject(const int objid);

	virtual void DrawPickingObjectFace(const int objid);

	virtual void DrawPickingObjectLine(const int objid);

	virtual int GetPickableElementCount(const int)
	{
		return 1;
	}

	virtual const char *Description(void) const
	{
		return "Point";
	}


	virtual int PolygonCount(void)
	{
		return 0;
	}

	virtual int TriangleCount(void)
	{
		return 0;
	}

	virtual void exportFile(FILE *fp, const char *format, const double * matrix=NULL);

};






#endif