#ifndef __OBJECT3D_H__

#define __OBJECT3D_H__


/********************************************************************** 
   Adaptive-sampled Voxelization Library (AVL) V1.0
   Copyright (C) 2002 Nan Zhang, State University of New York at Stony Brook

This file is part of AVL.

AVL is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

AVL is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Subdivide; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. 

  
File name:	object3d.h
Date:		Oct. 13, 2002
Author:		Nan Zhang
Version: 
History:   the second version, the first version is destroied in an accident

***********************************************************************/

#include <sysconf.h>
#include <vectorall.h>
#include <matrix2d.h>
#include <matrix2f.h>
#include <base_object.h>

//#include <drawparms.h>

class CGLDrawParms;

inline int SEARCH_KEY_POS(int* buff, const int len, const int key)
{
	int i;
	for (i=0; i<len; i++){
		if (buff[i]==key)
			return i;
	}
	assert(0);
	return -1;
}


class CAVLSample
{
public:
	double dist;
	Vector3f norm;
};


/***********************************************************
AxisAlignedBox class.
This is a box in 3-space, defined by min and max
corner vectors.  Many useful operations are defined on this
************************************************************/
class AxisAlignedBox 
{
public:
    //
    Vector3d minp, maxp;

public:
    // Constructors.
    AxisAlignedBox()
	{ 
		const double ZMAXFLOAT = 1e38;
		minp = Vector3d(ZMAXFLOAT, ZMAXFLOAT, ZMAXFLOAT);
		maxp = Vector3d(-ZMAXFLOAT, -ZMAXFLOAT, -ZMAXFLOAT);
	}

    //
    AxisAlignedBox(const Vector3d& Min, const Vector3d& Max)
    { 
        minp = Min;
        maxp = Max;
    }

    // Read-only access to min and max vectors.
    Vector3d Min() const;
    //
    Vector3d Max() const;

    /** Return whether the box is unbounded.  Unbounded boxes appear
        when unbounded objects such as quadric surfaces are included.
    */
    int Unbounded() const;

    // Expand the axis-aligned box to include the given object.
    void Include(const Vector3d& newpt);

    //
    void Include(const AxisAlignedBox& bbox);

    // Overlap returns 1 if the two axis-aligned boxes overlap.
    friend int Overlap(const AxisAlignedBox&, const AxisAlignedBox&);

    /** Returns the smallest axis-aligned box that includes all points
        inside the two given boxes.
    */
    friend AxisAlignedBox Union(const AxisAlignedBox& x, const AxisAlignedBox& y);

    // Returns the intersection of two axis-aligned boxes.
    friend AxisAlignedBox Intersect(const AxisAlignedBox& x, const AxisAlignedBox& y);

    //
    friend AxisAlignedBox Transform(const AxisAlignedBox& box, const Matrix2d& tform);

    //
    //virtual double SurfaceArea() const;
    //
    //int ComputeMinMaxT(const Ray& ray, double *tmin, double *tmax) const;
    //
    //friend class Ray;
    friend class CObject3D;

};


inline int Overlap(const AxisAlignedBox& x, const AxisAlignedBox& y)
{
	int r;
    if (x.maxp.x < y.minp.x ||
		x.minp.x > y.maxp.x ||
		x.maxp.y < y.minp.y ){
		r = 0;
	}
	else if (x.minp.y > y.maxp.y ||
		     x.maxp.z < y.minp.z ||
		     x.minp.z > y.maxp.z) {
		     r= 0;
	}else{
		r = 1;
	}
    return r;
}

//
inline AxisAlignedBox Intersect(const AxisAlignedBox& x, const AxisAlignedBox& y)
{
    if (x.Unbounded())
		return y;
    else if (y.Unbounded())
		return x;

    AxisAlignedBox ret = x;
    if (Overlap(ret, y)) {
		Maximize(ret.minp, y.minp);
		Minimize(ret.maxp, y.maxp);
		return ret;
    }
    // Null intersection.
	return AxisAlignedBox(Vector3d(0,0,0), Vector3d(0,0,0));
}

//
inline AxisAlignedBox Union(const AxisAlignedBox& x, const AxisAlignedBox& y)
{
    Vector3d minv = x.minp;
    Vector3d maxv = x.maxp;
    Minimize(minv, y.minp);
    Maximize(maxv, y.maxp);
    return AxisAlignedBox(minv, maxv);
}

//
inline AxisAlignedBox Transform(const AxisAlignedBox& box, const Matrix2d& tform)
{

	const Vector3d brange = Vector3d(MAXFLOAT, MAXFLOAT, MAXFLOAT);
    AxisAlignedBox ret(brange, -brange);
	/*
    ret.Include(Vector3d(box.minp.x, box.minp.y, box.minp.z)*tform);
    ret.Include(Vector3d(box.minp.x, box.minp.y, box.maxp.z)*tform);
    ret.Include(Vector3d(box.minp.x, box.maxp.y, box.minp.z)*tform);
    ret.Include(Vector3d(box.minp.x, box.maxp.y, box.maxp.z)*tform);
    ret.Include(Vector3d(box.maxp.x, box.minp.y, box.minp.z)*tform);
    ret.Include(Vector3d(box.maxp.x, box.minp.y, box.maxp.z)*tform);
    ret.Include(Vector3d(box.maxp.x, box.maxp.y, box.minp.z)*tform);
    ret.Include(box.maxp);
	*/
	assert(0);
    return ret;
}



inline Vector3d AxisAlignedBox::Min() const
{
    return minp;
}

inline Vector3d AxisAlignedBox::Max() const
{
    return maxp;
}

/*
inline int AxisAlignedBox::Unbounded() const
{
	const double KMIN = (double)(-MAXFLOAT);
	const double KMAX = (double)MAXFLOAT;
    return minp == Vector3d(KMAX, KMAX, KMAX) || maxp == Vector3d(KMIN, KMIN, KMIN);
}
*/

inline void AxisAlignedBox::Include(const Vector3d& newpt)
{
    Minimize(minp, newpt);
    Maximize(maxp, newpt);
}

inline void AxisAlignedBox::Include(const AxisAlignedBox& bbox)
{
    Minimize(minp, bbox.minp);
    Maximize(maxp, bbox.maxp);
}



class CObject3DBase: public CObjectBase
{
private:
	AxisAlignedBox m_box;

public:
    // Constructors set flags and color.
    CObject3DBase(void): CObjectBase()
	{
		m_box = AxisAlignedBox(Vector3d(0,0,0), Vector3d(1,1,1));
	}

	virtual ~CObject3DBase(void){}

	virtual const char *Description(void) const
	{
		return NULL;
	}

	virtual void SetCustomizedObjectName(const int key)
	{
		//set the object name;
		const char *des = this->Description();
		char namebuff[20]="";
		sprintf(namebuff, "O%02d%s", key, des);
		this->SetObjectName(namebuff);
	}

	virtual void GetBoundingBox(AxisAlignedBox & box)
	{
		box = m_box;
	}

	virtual void SetBoundingBox(const AxisAlignedBox &box)
	{
		m_box = box;
	}

};





//
class CObject3D: public CObject3DBase
{

public:
    // Constructors set flags and color.
    CObject3D(void): CObject3DBase()
	{
	}

    // Make destructor explicitly virtual so it can be overloaded.
    virtual ~CObject3D()
	{ 
	}

    /** NearestInt returns 1 if the ray hits the object; it passes back
        the parameter of intersection in t.  maxt specifies the maximum
        allowable value for the parameter of intersection; if the only 
        intersections with the object are of greater parameter than maxt,
        no intersection is returned.
    */

    // Returns the normal to the object at the given intersection point.
    virtual Vector3d FindNormal(const Vector3d& p) 
	{   
		Vector3d n(1, 0, 0);
		return n;
	}

    // Apply an arbitrary 3D 4x4 matrix to the object.
    virtual void ApplyTransform(const Matrix2d& tform){ }

	//
	virtual int LoadFile(const char *fname) { return true; }

	//
	//virtual int LoadFile(FILE *fp) { return true; }

	//
	virtual int Preprocess(void){ return true; }

	//
	virtual int initForSampling(int& auxint, void*& auxpointer){ return true; }

	//
	virtual int initForRendering(int& auxint, void*& auxpointer){ return true; }

	//
	virtual CAVLSample Distance(const Vector3d& p)
	{
		assert(0);
		CAVLSample s;
        s.dist = 0;
        s.norm = Vector3f(0,0,0);
		return s;
	}

    /** Return a measure of how far inside the object the given point is.
        This distance is 0 on the object's surface, negative if inside the
        object, or positive if outside the object.  If the number is
        positive or negative, it should reflect how far inside or outside
        the object the given point is.
        This function is used strictly by the SoftObject hypertexture class.
    */
    virtual double PtDistance(const Vector3d& v)
	{
		assert(0);
		return 0; 
	}

	virtual void glDraw(const CGLDrawParms &drawparms){};

	virtual void* getMeshRepresentation(bool &lock)
	{
		assert(0);
		lock = false;
		return NULL;
	};


	//object split
	virtual int ObjectCount(int objidbuff[], const int) // bufflen)
	{
		objidbuff[0]=0;
		return 1;
	}

	virtual int PolygonCount(void)
	{
		return 0;
	}

	virtual int TriangleCount(void)
	{
		return 0;
	}

	virtual int SplitIntoSubObj(const int objidbuff[], const int bufflen, CObject3D* obj[])
	{
		assert(0);
		return 1;
	}

	virtual void DrawPickingObject(const int objid)
	{

	}

	virtual void DrawPickingObjectFace(const int objid)
	{

	}

	virtual void DrawPickingObjectLine(const int objid)
	{

	}

	virtual int GetPickableElementCount(const int ptype)
	{
		return 0;
	}

	virtual void GetPickedLine(const int eid, Vector3d & v0, Vector3d &v1)
	{
		v1 = v0 = Vector3d(0,0,0);
	}

	virtual void GetPickedLine(const int eid, int & v0, int &v1)
	{
		v1 = v0 = -1;
	}

	virtual void GetPickedPlane(const int eid, Vector3d v[4])
	{
		//v.x = v.y = v.z = v.w = 0;
	}


	virtual void CopyAttribNames2DrawParms(CGLDrawParms &drawparms)
	{
	}

	//export data using the specified format and transform
	virtual void exportFile(FILE *fp, const char *format, const double * matrix=NULL)
	{
		assert(0);
	}

	virtual void applyTranslation(const double& tx, const double& ty, const double& tz)
	{
		assert(0);
	}

	virtual void applyScaling(const double& tx, const double& ty, const double& tz)
	{
		assert(0);
	}

};



#define CAVLObject3D CObject3D 

#endif //__OBJECT3D_H__
