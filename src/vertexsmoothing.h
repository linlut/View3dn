//Vertexinfo.h

#ifndef __INC_VERTEXSMOOTHING_H__
#define __INC_VERTEXSMOOTHING_H__

#include <sysconf.h>
#include <memmgr.h>
#include <vectorall.h>


#define VERTEX_AT_FREE				0
#define VERTEX_AT_BOUNDARY_LINE		1
#define VERTEX_AT_BOUNDARY_PLANE	2
#define VERTEX_AT_FIXED				10

class CGeneratedVertex
{
public:
	Vector3f v;
	Vector3f t;
	Vector3f norm;
	int attr;

	CGeneratedVertex(){};

	CGeneratedVertex(const Vector3f& V, const Vector3f& T, const Vector3f& N, const int ATTR)
	{
		v=V, t=T, attr=ATTR;
		norm = N;
	}
};


#endif