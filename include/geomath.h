//*********************************************************
//
//File: geomath.h
//
//*********************************************************


#ifndef __INC_GEOMATH_H__
#define __INC_GEOMATH_H__

#include <vectorall.h>
#include <matrix.h>

//shape classification
#define AVL_TRIANGLE	0
#define AVL_LINE    1


//Note the hex vertex order is different !!
inline void hexConnectivityToTetConnectivity(const Vector8i &hex, Vector4i tet[6])
{
	static Vector4i tab[6]={
		Vector4i(0,1,2,4), 
		Vector4i(4,5,1,2), 
		Vector4i(4,6,5,2), 
		Vector4i(1,3,2,5), 
		Vector4i(5,7,3,6), 
		Vector4i(2,3,6,5)
	};

	//copy and swtich vertices
	const Vector8i *p = &hex;
	const int x[8]={p->x, p->y, p->w, p->z, p->x1, p->y1, p->w1, p->z1};

	//create tets
	Vector4i *u = &tab[0];
	tet[0]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
	u = &tab[1];
	tet[1]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
	u = &tab[2];
	tet[2]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
	u = &tab[3];
	tet[3]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
	u = &tab[4];
	tet[4]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
	u = &tab[5];
	tet[5]=Vector4i(x[u->x], x[u->y], x[u->z], x[u->w]);
}


inline double computeTetrahedronVolume(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d)
{
	const double &x1 = a.x, &y1=a.y, &z1=a.z;
	const double &x2 = b.x, &y2=b.y, &z2=b.z;
	const double &x3 = c.x, &y3=c.y, &z3=c.z;
	const double &x4 = d.x, &y4=d.y, &z4=d.z;
	double A1,A2,A3,A4,value;

	A1=x2*(y3*z4-y4*z3)-x3*(y2*z4-y4*z2)+x4*(y2*z3-y3*z2);
	A2=x1*(y3*z4-y4*z3)-x3*(y1*z4-y4*z1)+x4*(y1*z3-y3*z1);
	A3=x1*(y2*z4-y4*z2)-x2*(y1*z4-y4*z1)+x4*(y1*z2-y2*z1);
	A4=x1*(y2*z3-y3*z2)-x2*(y1*z3-y3*z1)+x3*(y1*z2-y2*z1);
	value=(A1-A2+A3-A4)/6;
	return fabs(value);
}


inline double computeHexahedronVolume(const Vector3d *pVertex, const Vector8i &hex)
{
	double tvol = 0;
	Vector4i tet[6];
	//split the hex element into 6 tets
	hexConnectivityToTetConnectivity(hex, tet);
	//compute the sum of volume of the 6 tets
	for (int i=0; i<6; i++){
		const Vector4i *p = &tet[i];
		const Vector3d& a = pVertex[p->x];
		const Vector3d& b = pVertex[p->y];
		const Vector3d& c = pVertex[p->z];
		const Vector3d& d = pVertex[p->w];
		tvol+=computeTetrahedronVolume(a, b, c, d);
	}
	return tvol;
}



///
/// ********************************something for triangle**************************************
///
inline void compute_triangle_areanormal(
	const Vector3d& a, const Vector3d& b, const Vector3d& c, Vector3d& N)
{
	const Vector3d v1= b-a;
	const Vector3d v2= c-a;
	N = CrossProd(v1, v2);
}

inline void compute_quad_areanormal(
	const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d &d, Vector3d& N)
{
	const Vector3d v1= c-a;
	const Vector3d v2= d-b;
	N = CrossProd(v1, v2);
}

inline Vector3d compute_triangle_normal(const Vector3d& a, const Vector3d& b, const Vector3d& c)
{
	const Vector3d v1= b-a;
	const Vector3d v2= c-a;
	const Vector3d v = CrossProd(v1, v2);
	return Normalize(v);
}

inline Vector3d compute_quad_normal(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d)
{
	const Vector3d v1= c-a;
	const Vector3d v2= d-b;
	const Vector3d v = CrossProd(v1, v2);
	return Normalize(v);
}

inline Vector3f compute_triangle_normal(const Vector3f& a, const Vector3f& b, const Vector3f& c)
{
	Vector3f v1= b-a;
	Vector3f v2= c-a;
	Vector3f v = CrossProd(v1, v2);
	return Normalize(v);
}

inline double triangle_area(const Vector3d& a, const Vector3d& b, const Vector3d& c)
{
	Vector3d e0 = b - a;
	Vector3d e1 = c - a;
	Vector3d r = CrossProd(e0, e1);
	double A = Magnitude(r);
	A*=0.5;
	return A;
}

inline double quad_area(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d)
{
	double A = triangle_area(a, b, c);
	A += triangle_area(a, c, d);
	return A;
}

inline float triangle_area(const Vector3f& a, const Vector3f& b, const Vector3f& c)
{
	Vector3f e0 = b - a;
	Vector3f e1 = c - a;
	Vector3f r = CrossProd(e0, e1);
	float A = Magnitude(r);
	A*=0.5;
	return A;
}

inline void barycentric_coor(const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p, Vector3d &t)
{
	const float a0 = triangle_area(p, p1, p2);
	const float a1 = triangle_area(p, p2, p0);
	const float a2 = triangle_area(p, p0, p1);
	const float a = 1.0f/(a0+a1+a2);
	t.x = a0*a;
	t.y = a1*a;
	t.z = a2*a;
}


inline double triangle_contour_R(const Vector3d& v0, const Vector3d& v1, const Vector3d& v2)
{
	double a2, b2, c2, a, b, cosa, sina, R;
	a2 = Magnitude2(v0-v1);
	b2 = Magnitude2(v0-v2);
	c2 = Magnitude2(v1-v2);
	a= sqrt(a2);
	b =sqrt(b2);
	cosa = (a2+b2-c2)/(2*a*b);
	sina = sqrt(1-cosa*cosa);
	R= a/ sina;
	return R;
}


///
///!!!Return the cos^2 of the minimal triangle angle
///input is the edge length*2, since we want to avoid sqrt computations
///
inline float computeTriangleMinAngleCos2(const float aa2, const float bb2, const float cc2)
{
	float l[3], minlen, cosa2;
	float a2, b2, c2, t;
	int index;

	l[0]=aa2, l[1]=bb2, l[2]=cc2;
	minlen=l[0], index=0;
	if (minlen>l[1]) minlen=l[1], index = 1;
	if (minlen>l[2]) minlen=l[2], index = 2;

	a2 = l[index];
	b2 = l[(index+1)%3];
	c2 = l[(index+2)%3];
	t = b2+c2-a2;
	cosa2=0.25f * t * t / (b2 * c2);
	return cosa2;
}



inline 
int compute_triangle_normal(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, int &trishape, Vector3f &norm)
{
	Vector3f d1, d2, d3;
	const double TOL1 = 1e-10;
	float t;

	//
	trishape = AVL_TRIANGLE;

	//compute normal first
	d1 = v1 - v0;
	d2 = v2 - v1;
	d3 = v0 - v2;
	if ((fabs(d1.x)+fabs(d1.y)+fabs(d1.z)< TOL1)||
		(fabs(d2.x)+fabs(d2.y)+fabs(d2.z)< TOL1)||
		(fabs(d3.x)+fabs(d3.y)+fabs(d3.z)< TOL1)){
		trishape = AVL_LINE;
		return 0; //two points are the same
	}

	//normalize the normal;
	norm = CrossProd(d1, d2);
	t = sqrt(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);
	if (t<TOL1){
		trishape = AVL_LINE;
		return 0;
	}
	t = 1.0f/t;
	norm.x*=t, norm.y*=t, norm.z*=t;
	return 1;
}


/*
inline 
int test_triangle_normal(Vector3f* v[3], const int edgeid, int &trishape, Vector3f &norm)
{
	int i=edgeid, axis, r, k;
	Vector3I edge_info;
	const float TOL1 = 1e-10f;
	float t;

	if (!compute_triangle_normal(*v[0], *v[1], *v[2], trishape, norm))
		return 0;

	//get the axis projection
	k = i;
	if (i>=12) k -= 12;
	axis = CAVLCubeInfo::getCubeEdgeAxis(k);
	t = norm[axis];
	if (fabs(t)>TOL1){
		if (i<12){
			if (t < 0) 
				r = 0;
			else
				r = 1;
		}
		else{
			if (t < 0)
				r = 1;
			else
				r = 0;
		}
	}
	else{
#ifdef _DEBUG
		printf("TOO small gradient,hard to know!\n");
#endif
		r = 1;
	}
	return r;
}
*/

inline 
int test_triangle_normal(Vector3f* v[3], const int edgeid, int &trishape, const int axis, Vector3f &norm)
{
	int i=edgeid, r;
	Vector3I edge_info;
	const float TOL1 = 1e-10f;
	float t;

	if (!compute_triangle_normal(*v[0], *v[1], *v[2], trishape, norm))
		return 0;

	//get the axis projection
	/*
	k = i;
	if (i>=12) k -= 12;
	axis = CAVLCubeInfo::getCubeEdgeAxis(k);
	*/
	t = norm[axis];
	if (fabs(t)>TOL1){
		if (i<12){
			if (t < 0) 
				r = 0;
			else
				r = 1;
		}
		else{
			if (t < 0)
				r = 1;
			else
				r = 0;
		}
	}
	else{
#ifdef _DEBUG
		printf("TOO small gradient,hard to know!\n");
#endif
		r = 1;
	}
	return r;
}


///***************************Something for quad*********************************

///
///Test whether the quad is convex, return the vertex id to be split
///
inline 
int isQuadConvex(const Vector3f edge[4], int &vertex)
{
	Vector3f n1, n2, n3, n0;
	int rtval=false;

	vertex = 0;
	n1 = CrossProd(edge[0], edge[1]);
	n2 = CrossProd(edge[1], edge[2]);
	n3 = CrossProd(edge[2], edge[3]);

	if (DotProd(n1, n2)<=0){
		if (DotProd(n1, n3)<=0){
			vertex = 1;
		}
		else{
			vertex = 2;
		}
	}
	else{
		n0 = CrossProd(edge[3], edge[0]);
		if (DotProd(n3, n0)<=0){
			if (DotProd(n2, n3)<=0){
				vertex=3;
			}
			else{
				vertex=0;
			}
		}
		else{
			rtval = true;
		}
	}
	return rtval;
}



//Input: a convex quad (v0, v1, v2, v3
// return: true:  use the split (v0,v1,v3),(v1,v2,v3);
//         false: use the split (v0,v1,v2),(v0,v2,v3);
inline
bool QuadShapeSplitGood(Vector3f* v[4], Vector3f evec[4])
{
	float len2[4], diag[2];
	float a1, a2, b1, b2;
	float a, b;

	len2[0]= (float)Magnitude2(evec[0]); 
	len2[1]= (float)Magnitude2(evec[1]); 
	len2[2]= (float)Magnitude2(evec[2]); 
	len2[3]= (float)Magnitude2(evec[3]); 

	diag[0]= (float)Distance2(*v[0], *v[2]);
	diag[1]= (float)Distance2(*v[1], *v[3]);

	a1 = computeTriangleMinAngleCos2(len2[0], len2[1], diag[0]);
	a2 = computeTriangleMinAngleCos2(len2[2], len2[3], diag[0]);
	b1 = computeTriangleMinAngleCos2(len2[0], diag[1], len2[3]);
	b2 = computeTriangleMinAngleCos2(len2[1], len2[2], diag[1]);
	a = _MAX_(a1, a2);
	b = _MAX_(b1, b2);
	if (a<b)
		return true;
	else
		return false;
}


///
///*****************************Something for line/triangle intersection*******************************
///
inline int XLINE_TRIANGLE_INTSECTION(const Vector3f& p0, const Vector3f& n, const float d, Vector3f &intp)
{
	const double TOL1=1e-12;
	double dotn = n.x;
	if (fabs(dotn)<TOL1)
		return false;
	const float t = (float)(-((d+DotProd(p0, n))/dotn));
	intp.x = p0.x + t;
	intp.y = p0.y;
	intp.z = p0.z;
	return true;
}


inline int YLINE_TRIANGLE_INTSECTION(const Vector3f& p0, const Vector3f& n, const float d, Vector3f &intp)
{
	const double TOL1=1e-12;
	double dotn = n.y;
	if (fabs(dotn)<TOL1)
		return false;
	double t = -((d+DotProd(p0, n))/dotn);
	intp.x = p0.x;
	intp.y = p0.y + (float)t;
	intp.z = p0.z;
	return true;
}


inline int ZLINE_TRIANGLE_INTSECTION(const Vector3f& p0, const Vector3f& n, const float d, Vector3f &intp)
{
	const double TOL1=1e-12;
	double dotn = n.z;
	if (fabs(dotn)<TOL1)
		return false;
	const float t = (float)(-((d+DotProd(p0, n))/dotn));
	intp.x = p0.x;
	intp.y = p0.y;
	intp.z = p0.z + t;
	return true;
}


inline int LINE_TRIANGLE_INTSECTION(const Vector3f& p0, const Vector3f& nl, const Vector3f& np, const float d, Vector3f &intp)
{
	float dotn;
	float t;

	dotn = DotProd(nl, np);
	if (fabs(dotn)<1e-15f)
		return false;
	t = -((d+DotProd(p0, np))/dotn);
	intp.x = p0.x+t*nl.x;
	intp.y = p0.y+t*nl.y;
	intp.z = p0.z+t*nl.z;
	return true;
}


inline double getAngle(const double x, const double y)
{
	double a =atan2(y, x);
	if (a<0) a+=2*M_PI;
	return a;
}

//classify two point with oregin bounded by two intersection planes;
inline int XAxis_Points2D_InSameRegion(const Vector3f& n1, const Vector3f& n2)
{
	double angle1;
	double angle2;
	double a3;
	double t;
	const double TOL1=1e-6;

	angle1 =getAngle(n1.x, n1.y);
	angle2 =getAngle(n2.x, n2.y);

	if (angle1>angle2){ //swap
		t = angle1;
		angle1 = angle2;
		angle2 = t;
	}
	if (angle1+angle2<TOL1)
		return 0;
	if (fabs(angle1-PI)+fabs(angle2-PI)<TOL1)
		return 0;

	a3 =PI;
	if (a3>=angle1 && a3<angle2)
		return 0;
	return 1;
}



///
///*****************************Point/cell relation*********************************
///

inline bool TEST_POINT_IN_CELL(const int level, const int x, const int y, const int z, const Vector3f& c)
{
	const float size=(float)(1<<level);
	float t;

	t= size*c.x - x;
	if (t<0 || t>=1) return false;
	t= size*c.y - y;
	if (t<0 || t>=1) return false;
	t= size*c.z - z;
	if (t<0 || t>=1) return false;
	return true;
}

inline bool TEST_POINT_IN_CELL(const int level, const int x, const int y, const int z, const float factor, const Vector3f& c)
{
	float size=(float)(1<<level);
	float t;
	float tmin, tmax;

	tmin = -factor;
	tmax = 1.0f+factor;

	t= size*c.x - x;
	if (t<tmin || t>tmax) return false;
	t= size*c.y - y;
	if (t<tmin || t>tmax) return false;
	t= size*c.z - z;
	if (t<tmin || t>tmax) return false;
	return true;
}


inline 
Vector3f GET_POINT_POS(const int x,  const int y, const int z, const int i, const double& cubesize)
{
	int x2, y2, z2;
	Vector3f p;

	x2 = x+(i&0x1); 
	y2 = y+((i>>1)&0x1); 
	z2 = z+((i>>2)&0x1);
	p.x = (float)(x2*cubesize);
	p.y = (float)(y2*cubesize);
	p.z = (float)(z2*cubesize);
	return p;
}


#endif
