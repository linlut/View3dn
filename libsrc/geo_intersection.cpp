//FILE: geo_intersection.cpp

#include <geo_intersection.h>

// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test equality
//            != to test inequality
//            (Vector)0 = (0,0,0)         (null vector)
//            Point  = Point ± Vector
//            Vector = Point - Point
//            Vector = Scalar * Vector    (scalar product)
//            Vector = Vector * Vector    (cross product)
//    Line and Ray and Segment with defining points {Point P0, P1;}
//        (a Line is infinite, Rays and Segments start at P0)
//        (a Ray extends beyond P1, but a Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector n;}
//    Triangle with defining vertices {Point V0, V1, V2;}
//    Polyline and Polygon with n vertices {int n; Point *V;}
//        (a Polygon has V[n]=V[0])
//===================================================================

#define SMALL_NUM  0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v) ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)

// intersect_RayTriangle(): intersect a ray with a 3D triangle
//    Input:  a ray R, and a triangle T
//    Output: *I = intersection point (when it exists)
//    Return: -1 = triangle is degenerate (a segment or point)
//             0 = disjoint (no intersect)
//             1 = intersect in unique point I1
//             2 = are in the same plane
class Triangle
{
public:
	Vector3d V0, V1, V2;
};

inline int intersect_RayTriangle(const Vector3d &raypos, const Vector3d &dir, Triangle T, Vector3d* I, double *tt)
{
    Vector3d u, v, n;           // triangle vectors
    Vector3d w0, w;				// ray vectors
    double r, a, b;             // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = T.V1 - T.V0;
    v = T.V2 - T.V0;
    n = u * v;             // cross product
    if (n == Vector3d(0))            // triangle is degenerate
        return -1;                 // do not deal with this case

    w0 = raypos - T.V0;
    a = -dot(n,w0);
    b = dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is parallel to triangle plane
        if (a == 0)                // ray lies in triangle plane
            return 2;
        else return 0;             // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                   // ray goes away from triangle
        return 0;                  // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
	*tt = r;

    *I = raypos + r * dir;           // intersect point of ray and plane

    // is I inside T?
    double uu, uv, vv, wu, wv, D;
    uu = dot(u,u);
    uv = dot(u,v);
    vv = dot(v,v);
    w = *I - T.V0;
    wu = dot(w,u);
    wv = dot(w,v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    double s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)        // I is outside T
        return 0;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
        return 0;

    return 1;                      // I is in T
}

/*References
Didier Badouel, "An Efficient Ray-Polygon Intersection" in Graphics Gems (1990)

Francis Hill, "The Pleasures of 'Perp Dot' Products" in Graphics Gems IV (1994)
[Note: the first critical definition has a typo, and should be: a^ = (-ay, ax).]

Tomas Moller & Eric Haines, Real-Time Rendering, Chapter 10 "Intersection Test Methods" (1999)

Tomas Moller & Ben Trumbore, "Fast, Minimum Storage Ray-Triangle Intersection",  J. Graphics Tools 2(1), 21-28 (1997)

Joseph O'Rourke, Computational Geometry in C (2nd Edition), Section 7.3 "Segment-Triangle Intersection" (1998)

J.P. Snyder and A.H. Barr, "Ray Tracing Complex Models Containing Surface Tessellations", ACM Comp Graphics 21, (1987)
 */


int intersect_RayTriangle(
	const Vector3f &_raypos, const Vector3f& _raydir,			//ray parameters
	const Vector3f &P0, const Vector3f &P1, const Vector3f &P2,	//triangle points
	Vector3f &intp, float &t_parm)	
{
	Triangle t; 
	Vector3d raypos, raydir, *q, ip;
	double tt;
	raypos = Vector3d(_raypos.x, _raypos.y, _raypos.z);
	raydir = Vector3d(_raydir.x, _raydir.y, _raydir.z);
	q=&t.V0; q->x=P0.x; q->y=P0.y; q->z=P0.z;
	q=&t.V1; q->x=P1.x; q->y=P1.y; q->z=P1.z;
	q=&t.V2; q->x=P2.x; q->y=P2.y; q->z=P2.z;

	int f = intersect_RayTriangle(raypos, raydir, t, &ip, &tt);
	if (f!=1) return 0;
	t_parm = (float)tt;
	intp.x = (float)ip.x; intp.y = (float)ip.y; intp.z = (float)ip.z;
	return 1;
}

