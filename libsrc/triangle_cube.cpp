#include <triangle_cube.h>


/*___________________________________________________________________________*/

/* Which of the six face-plane(s) is point P outside of? */

inline int face_plane(const Point3& p)
{
int outcode;

   outcode = 0;
   if (p.x >  .5) outcode |= 0x01;
   if (p.x < -.5) outcode |= 0x02;
   if (p.y >  .5) outcode |= 0x04;
   if (p.y < -.5) outcode |= 0x08;
   if (p.z >  .5) outcode |= 0x10;
   if (p.z < -.5) outcode |= 0x20;
   return(outcode);
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* Which of the twelve edge plane(s) is point P outside of? */

inline int bevel_2d(const Point3& p)
{
int outcode;

   outcode = 0;
   if ( p.x + p.y > 1.0) outcode |= 0x001;
   if ( p.x - p.y > 1.0) outcode |= 0x002;
   if (-p.x + p.y > 1.0) outcode |= 0x004;
   if (-p.x - p.y > 1.0) outcode |= 0x008;
   if ( p.x + p.z > 1.0) outcode |= 0x010;
   if ( p.x - p.z > 1.0) outcode |= 0x020;
   if (-p.x + p.z > 1.0) outcode |= 0x040;
   if (-p.x - p.z > 1.0) outcode |= 0x080;
   if ( p.y + p.z > 1.0) outcode |= 0x100;
   if ( p.y - p.z > 1.0) outcode |= 0x200;
   if (-p.y + p.z > 1.0) outcode |= 0x400;
   if (-p.y - p.z > 1.0) outcode |= 0x800;
   return(outcode);
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* Which of the eight corner plane(s) is point P outside of? */

inline int bevel_3d(const Point3& p)
{
int outcode;

   outcode = 0;
   if (( p.x + p.y + p.z) > 1.5) outcode |= 0x01;
   if (( p.x + p.y - p.z) > 1.5) outcode |= 0x02;
   if (( p.x - p.y + p.z) > 1.5) outcode |= 0x04;
   if (( p.x - p.y - p.z) > 1.5) outcode |= 0x08;
   if ((-p.x + p.y + p.z) > 1.5) outcode |= 0x10;
   if ((-p.x + p.y - p.z) > 1.5) outcode |= 0x20;
   if ((-p.x - p.y + p.z) > 1.5) outcode |= 0x40;
   if ((-p.x - p.y - p.z) > 1.5) outcode |= 0x80;
   return(outcode);
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* Test the point "alpha" of the way from P1 to P2 */
/* See if it is on a face of the cube              */
/* Consider only faces in "mask"                   */

inline int check_point(const Point3& p1, const Point3& p2, const float alpha, const int mask)
{
Point3 plane_point;

   plane_point.x = LERP(alpha, p1.x, p2.x);
   plane_point.y = LERP(alpha, p1.y, p2.y);
   plane_point.z = LERP(alpha, p1.z, p2.z);
   return(face_plane(plane_point) & mask);
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* Compute intersection of P1 --> P2 line segment with face planes */
/* Then test intersection point to see if it is on cube face       */
/* Consider only face planes in "outcode_diff"                     */
/* Note: Zero bits in "outcode_diff" means face line is outside of */

inline int check_line(const Point3& p1, const Point3& p2, const int outcode_diff)
{

   if ((0x01 & outcode_diff) != 0)
      if (check_point(p1,p2,( .5-p1.x)/(p2.x-p1.x),0x3e) == INSIDE) return(INSIDE);
   if ((0x02 & outcode_diff) != 0)
      if (check_point(p1,p2,(-.5-p1.x)/(p2.x-p1.x),0x3d) == INSIDE) return(INSIDE);
   if ((0x04 & outcode_diff) != 0) 
      if (check_point(p1,p2,( .5-p1.y)/(p2.y-p1.y),0x3b) == INSIDE) return(INSIDE);
   if ((0x08 & outcode_diff) != 0) 
      if (check_point(p1,p2,(-.5-p1.y)/(p2.y-p1.y),0x37) == INSIDE) return(INSIDE);
   if ((0x10 & outcode_diff) != 0) 
      if (check_point(p1,p2,( .5-p1.z)/(p2.z-p1.z),0x2f) == INSIDE) return(INSIDE);
   if ((0x20 & outcode_diff) != 0) 
      if (check_point(p1,p2,(-.5-p1.z)/(p2.z-p1.z),0x1f) == INSIDE) return(INSIDE);
   return(OUTSIDE);
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/* Test if 3D point is inside 3D triangle */

inline int point_triangle_intersection(const Point3& p, const Triangle3& t)
{
 int sign12,sign23,sign31;
 Point3 vect12,vect23,vect31,vect1h,vect2h,vect3h;
 Point3 cross12_1p,cross23_2p,cross31_3p;

/* First, a quick bounding-box test:                               */
/* If P is outside triangle bbox, there cannot be an intersection. */

   if (p.x > MAX3(t.v1.x, t.v2.x, t.v3.x)) return(OUTSIDE);  
   if (p.y > MAX3(t.v1.y, t.v2.y, t.v3.y)) return(OUTSIDE);
   if (p.z > MAX3(t.v1.z, t.v2.z, t.v3.z)) return(OUTSIDE);
   if (p.x < MIN3(t.v1.x, t.v2.x, t.v3.x)) return(OUTSIDE);
   if (p.y < MIN3(t.v1.y, t.v2.y, t.v3.y)) return(OUTSIDE);
   if (p.z < MIN3(t.v1.z, t.v2.z, t.v3.z)) return(OUTSIDE);

/* For each triangle side, make a vector out of it by subtracting vertexes; */
/* make another vector from one vertex to point P.                          */
/* The crossproduct of these two vectors is orthogonal to both and the      */
/* signs of its X,Y,Z components indicate whether P was to the inside or    */
/* to the outside of this triangle side.                                    */

   SUB(t.v1, t.v2, vect12)
   SUB(t.v1,    p, vect1h);
   CROSS(vect12, vect1h, cross12_1p)
   sign12 = SIGN3(cross12_1p);      /* Extract X,Y,Z signs as 0..7 or 0...63 integer */

   SUB(t.v2, t.v3, vect23)
   SUB(t.v2,    p, vect2h);
   CROSS(vect23, vect2h, cross23_2p)
   sign23 = SIGN3(cross23_2p);

   SUB(t.v3, t.v1, vect31)
   SUB(t.v3,    p, vect3h);
   CROSS(vect31, vect3h, cross31_3p)
   sign31 = SIGN3(cross31_3p);

/* If all three crossproduct vectors agree in their component signs,  */
/* then the point must be inside all three.                           */
/* P cannot be OUTSIDE all three sides simultaneously.                */

   /* this is the old test; with the revised SIGN3() macro, the test
    * needs to be revised. */
#ifdef OLD_TEST
   if ((sign12 == sign23) && (sign23 == sign31))
      return(INSIDE);
   else
      return(OUTSIDE);
#else
   return ((sign12 & sign23 & sign31) == 0) ? OUTSIDE : INSIDE;
#endif
}

/*. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

/**********************************************/
/* This is the main algorithm procedure.      */
/* Triangle t is compared with a unit cube,   */
/* centered on the origin.                    */
/* It returns INSIDE (0) or OUTSIDE(1) if t   */
/* intersects or does not intersect the cube. */
/**********************************************/

int TriangleOutsideCube(const Triangle3& t)
{
	int v1_test,v2_test,v3_test;
	float d;

/* First compare all three vertexes with all six face-planes */
/* If any vertex is inside the cube, return immediately!     */

   if ((v1_test = face_plane(t.v1)) == INSIDE) return(INSIDE);
   if ((v2_test = face_plane(t.v2)) == INSIDE) return(INSIDE);
   if ((v3_test = face_plane(t.v3)) == INSIDE) return(INSIDE);

/* If all three vertexes were outside of one or more face-planes, */
/* return immediately with a trivial rejection!                   */

   if ((v1_test & v2_test & v3_test) != 0) return(OUTSIDE);

/* Now do the same trivial rejection test for the 12 edge planes */

   v1_test |= bevel_2d(t.v1) << 8; 
   v2_test |= bevel_2d(t.v2) << 8; 
   v3_test |= bevel_2d(t.v3) << 8;
   if ((v1_test & v2_test & v3_test) != 0) return(OUTSIDE);  

/* Now do the same trivial rejection test for the 8 corner planes */

   v1_test |= bevel_3d(t.v1) << 24; 
   v2_test |= bevel_3d(t.v2) << 24; 
   v3_test |= bevel_3d(t.v3) << 24; 
   if ((v1_test & v2_test & v3_test) != 0) return(OUTSIDE);   

/* If vertex 1 and 2, as a pair, cannot be trivially rejected */
/* by the above tests, then see if the v1-->v2 triangle edge  */
/* intersects the cube.  Do the same for v1-->v3 and v2-->v3. */
/* Pass to the intersection algorithm the "OR" of the outcode */
/* bits, so that only those cube faces which are spanned by   */
/* each triangle edge need be tested.                         */

   if ((v1_test & v2_test) == 0)
      if (check_line(t.v1,t.v2,v1_test|v2_test) == INSIDE) 
		  return(INSIDE);
   if ((v1_test & v3_test) == 0)
      if (check_line(t.v1,t.v3,v1_test|v3_test) == INSIDE) 
		  return(INSIDE);
   if ((v2_test & v3_test) == 0)
      if (check_line(t.v2,t.v3,v2_test|v3_test) == INSIDE) 
		  return(INSIDE);

/* By now, we know that the triangle is not off to any side,     */
/* and that its sides do not penetrate the cube.  We must now    */
/* test for the cube intersecting the interior of the triangle.  */
/* We do this by looking for intersections between the cube      */
/* diagonals and the triangle...first finding the intersection   */
/* of the four diagonals with the plane of the triangle, and     */
/* then if that intersection is inside the cube, pursuing        */
/* whether the intersection point is inside the triangle itself. */

/* To find plane of the triangle, first perform crossproduct on  */
/* two triangle side vectors to compute the normal vector.       */  

   //Point3 vect12,vect13;
   //SUB(t.v1,t.v2,vect12);
   //SUB(t.v1,t.v3,vect13);
   //CROSS(vect12,vect13,norm)
   const Point3 &norm= t.norm;
   d = norm.x * t.v1.x + norm.y * t.v1.y + norm.z * t.v1.z;

/* The normal vector "norm" X,Y,Z components are the coefficients */
/* of the triangles AX + BY + CZ + D = 0 plane equation.  If we   */
/* solve the plane equation for X=Y=Z (a diagonal), we get        */
/* -D/(A+B+C) as a metric of the distance from cube center to the */
/* diagonal/plane intersection.  If this is between -0.5 and 0.5, */
/* the intersection is inside the cube.  If so, we continue by    */
/* doing a point/triangle intersection.                           */
/* Do this for all four diagonals.                                */
/*
   hitpp.x = hitpp.y = hitpp.z = d / (norm.x + norm.y + norm.z);
   if (fabs(hitpp.x) <= 0.5)
      if (point_triangle_intersection(hitpp,t) == INSIDE) 
		  return(INSIDE);

   hitpn.z = -(hitpn.x = hitpn.y = d / (norm.x + norm.y - norm.z));
   if (fabs(hitpn.x) <= 0.5)
      if (point_triangle_intersection(hitpn,t) == INSIDE) 
		  return(INSIDE);

   hitnp.y = -(hitnp.x = hitnp.z = d / (norm.x - norm.y + norm.z));
   if (fabs(hitnp.x) <= 0.5)
      if (point_triangle_intersection(hitnp,t) == INSIDE) 
		  return(INSIDE);

   hitnn.y = hitnn.z = -(hitnn.x = d / (norm.x - norm.y - norm.z));
   if (fabs(hitnn.x) <= 0.5)
      if (point_triangle_intersection(hitnn,t) == INSIDE) 
		  return(INSIDE);
*/

   //project the triangle into 2D to use 2D triangle/point classification
	Point3 vertex[3], intp;
	t.get2DPointProjection(t.v1, vertex[0]);
	t.get2DPointProjection(t.v2, vertex[1]);
	t.get2DPointProjection(t.v3, vertex[2]);

	{
		Point3 hitpp;
		hitpp.x = hitpp.y = hitpp.z = d / (norm.x + norm.y + norm.z);
		if (fabs(hitpp.x) <= 0.5){
			t.get2DPointProjection(hitpp, intp);
			if (point_in_triangle2D(intp, vertex)==1)
  				return(INSIDE);
		}
	}

	{
		Point3 hitpn;
		hitpn.z = -(hitpn.x = hitpn.y = d / (norm.x + norm.y - norm.z));
		if (fabs(hitpn.x) <= 0.5){
			t.get2DPointProjection(hitpn, intp);
			if (point_in_triangle2D(intp, vertex)==1)
  				return(INSIDE);
		}
	}


	{
		Point3 hitnp;
		hitnp.y = -(hitnp.x = hitnp.z = d / (norm.x - norm.y + norm.z));
		if (fabs(hitnp.x) <= 0.5){
			t.get2DPointProjection(hitnp, intp);
			if (point_in_triangle2D(intp, vertex)==1)
  				return(INSIDE);
		}
	}

	{
		Point3 hitnn;
		hitnn.y = hitnn.z = -(hitnn.x = d / (norm.x - norm.y - norm.z));
		if (fabs(hitnn.x) <= 0.5){
			t.get2DPointProjection(hitnn, intp);
			if (point_in_triangle2D(intp, vertex)==1)
  				return(INSIDE);
		}
	}


/* No edge touched the cube; no cube diagonal touched the triangle. */
/* We're done...there was no intersection.                          */

	return(OUTSIDE);
}

