/////////////////////////////////////////////////////////////////////////////
/// MedVR Simulation
/// CREST, UMN (C)
/////////////////////////////////////////////////////////////////////////////
/// http://jgt.akpeters.com/papers/MollerTrumbore97/
//  Reference: Tomas Möller and Ben Trumbore, "Fast, Minimum Storage Ray-Triangle Intersection," journal of graphics tools", vol. 2(1), pages 21-28, 1997
/////////////////////////////////////////////////////////////////////////////

#ifndef __RAYTRI_H__
#define __RAYTRI_H__

#include "Defines.h"

#define RayTriMethod 1

#if RayTriMethod==1

	#define EPSILON 0.000001
	#define CROSS(dest,v1,v2) dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
	#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
	#define SUB(dest,v1,v2) dest[0]=v1[0]-v2[0]; dest[1]=v1[1]-v2[1]; dest[2]=v1[2]-v2[2];

	int
	intersect_triangle(CDFloat orig[3], CDFloat dir[3],
					   CDFloat vert0[3], CDFloat vert1[3], CDFloat vert2[3],
					   CDFloat *t, CDFloat *u, CDFloat *v)
	{
	   CDFloat edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
	   CDFloat det,inv_det;

	   /* find vectors for two edges sharing vert0 */
	   SUB(edge1, vert1, vert0);
	   SUB(edge2, vert2, vert0);

	   /* begin calculating determinant - also used to calculate U parameter */
	   CROSS(pvec, dir, edge2);

	   /* if determinant is near zero, ray lies in plane of triangle */
	   det = DOT(edge1, pvec);

	#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
	   if (det < EPSILON)
		  return 0;

	   /* calculate distance from vert0 to ray origin */
	   SUB(tvec, orig, vert0);

	   /* calculate U parameter and test bounds */
	   *u = DOT(tvec, pvec);
	   if (*u < 0.0 || *u > det)
		  return 0;

	   /* prepare to test V parameter */
	   CROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
	   *v = DOT(dir, qvec);
	   if (*v < 0.0 || *u + *v > det)
		  return 0;

	   /* calculate t, scale parameters, ray intersects triangle */
	   *t = DOT(edge2, qvec);
	   inv_det = 1.0 / det;
	   *t *= inv_det;
	   *u *= inv_det;
	   *v *= inv_det;
	#else                    /* the non-culling branch */
	   if (det > -EPSILON && det < EPSILON)
		 return 0;
	   inv_det = 1.0 / det;

	   /* calculate distance from vert0 to ray origin */
	   SUB(tvec, orig, vert0);

	   /* calculate U parameter and test bounds */
	   *u = DOT(tvec, pvec) * inv_det;
	   if (*u < 0.0 || *u > 1.0)
		 return 0;

	   /* prepare to test V parameter */
	   CROSS(qvec, tvec, edge1);

	   /* calculate V parameter and test bounds */
	   *v = DOT(dir, qvec) * inv_det;
	   if (*v < 0.0 || *u + *v > 1.0)
		 return 0;

	   /* calculate t, ray intersects triangle */
	   *t = DOT(edge2, qvec) * inv_det;
	#endif
	   return 1;
	}
#endif

#if RayTriMethod==2

	#define EPSILON 0.000001

	int
	intersect_triangle_barycentric(
	   CDFloat orig[3], CDFloat dir[3],
	   CDFloat vert0[3], CDFloat vert1[3], CDFloat vert2[3],
	   CDFloat planeq[4], int i1, int i2,
	   CDFloat *t, CDFloat *alpha, CDFloat *beta)
	{
	   CDFloat	dot, dot2;
	   CDFloat	point[2];
	   CDFloat	u0, v0, u1, v1, u2, v2;

	   /* is ray parallel to plane? */
	   dot = dir[0] * planeq[0] + dir[1] * planeq[1] + dir[2] * planeq[2];
	   if (dot < EPSILON && dot > -EPSILON)		/* or use culling check */
		  return 0;

	   /* find distance to plane and intersection point */
	   dot2 = orig[0] * planeq[0] +
		  orig[1] * planeq[1] + orig[2] * planeq[2];
	   *t = -(planeq[3] + dot2 ) / dot;
	   point[0] = orig[i1] + dir[i1] * *t;
	   point[1] = orig[i2] + dir[i2] * *t;

	   /* begin barycentric intersection algorithm */
	   u0 = point[0] - vert0[i1];
	   v0 = point[1] - vert0[i2];
	   u1 = vert1[i1] - vert0[i1];
	   u2 = vert2[i1] - vert0[i1];
	   v1 = vert1[i2] - vert0[i2];
	   v2 = vert2[i2] - vert0[i2];

	   /* calculate and compare barycentric coordinates */
	   if (u1 == 0) {		/* uncommon case */
		  *beta = u0 / u2;
		  if (*beta < 0 || *beta > 1)
			   return 0;
		  *alpha = (v0 - *beta * v2) / v1;
	   }
	   else {			/* common case, used for this analysis */
		  *beta = (v0 * u1 - u0 * v1) / (v2 * u1 - u2 * v1);
		  if (*beta < 0 || *beta > 1)
			   return 0;
		  *alpha = (u0 - *beta * u2) / u1;
	   }

	   if (*alpha < 0 || (*alpha + *beta) > 1.0)
		  return 0;

	   return 1;
	}
#endif

#endif //__RAYTRI_H__