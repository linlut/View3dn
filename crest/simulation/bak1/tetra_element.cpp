//FILE: thinsehll_element.cpp
#include <geomath.h>
#include <ztime.h>
#include "tetra_element.h"


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
static inline void GenReferencePlanes(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	double3x3& mat0, double3x3& mat1, double3x3& mat2, double3x3& mat3)
{
	Vector3d e10 = p1 - p0;
	Vector3d e32 = p3 - p2;
	e10.Normalize();
	e32.Normalize();
	//------------PLANE0----023----------------------
	{Vector3d &X = *((Vector3d*)(&mat0.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat0.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat0.x[6]));
	X = e32;
	Y = p0 - p2;
	Z = CrossProd(X, Y); Z.Normalize();
	Y = CrossProd(Z, X);}
	//------------PLANE1----123----------------------
	{Vector3d &X1 = *((Vector3d*)(&mat1.x[0]));
	Vector3d &Y1 = *((Vector3d*)(&mat1.x[3]));
	Vector3d &Z1 = *((Vector3d*)(&mat1.x[6]));
	X1 = e32;
	Y1 = p1 - p2; 
	Z1 = CrossProd(X1, Y1); Z1.Normalize();
	Y1 = CrossProd(Z1, X1);}
	//------------PLANE2---012-----------------------
	{Vector3d &X2 = *((Vector3d*)(&mat2.x[0]));
	Vector3d &Y2 = *((Vector3d*)(&mat2.x[3]));
	Vector3d &Z2 = *((Vector3d*)(&mat2.x[6]));
	X2 = e10;
	Y2 = p2 - p1;
	Z2 = CrossProd(X2, Y2); Z2.Normalize();
	Y2 = CrossProd(Z2, X2);}
	//------------PLANE3----013-----------------------
	{Vector3d &X2 = *((Vector3d*)(&mat3.x[0]));
	Vector3d &Y2 = *((Vector3d*)(&mat3.x[3]));
	Vector3d &Z2 = *((Vector3d*)(&mat3.x[6]));
	X2 = e10;
	Y2 = p3 - p0; 
	Z2 = CrossProd(X2, Y2); Z2.Normalize();
	Y2 = CrossProd(Z2, X2);}
}


//shear element composed of two triangles
void CTetra2Element::_initShearElements(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
{
	//compute volume for each triangle, each truss edge
	const double vol1 = computeTetrahedronVolume(p0, p1, p2, p3);
	const double trussvol = vol1*0.25;

	//construct the world/local transform matrix for the two triangles
	double3x3 mat0, mat1, mat2, mat3;
	GenReferencePlanes(p0, p1, p2, p3, mat0, mat1, mat2, mat3);
	m_shearelm[0].init(p0, p1, mat0, trussvol);
	m_shearelm[1].init(p0, p1, mat1, trussvol);
	m_shearelm[2].init(p2, p3, mat2, trussvol);
	m_shearelm[3].init(p2, p3, mat3, trussvol);
}


void CTetra2Element::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const CMeMaterialProperty &mtl, Vector3d F[4])
{
	Vector3d ff[4];			//global elastic force for each node i

	//gen. the world/local and local/world transformation matrices
	double3x3 rotmat0, rotmat1, rotmat2, rotmat3;
	GenReferencePlanes(p0, p1, p2, p3, rotmat0, rotmat1, rotmat2, rotmat3);

	//compute local forces for each shear element
	m_shearelm[0].computeNodalForce(p0, p1, rotmat0, mtl, ff[0]);
	m_shearelm[1].computeNodalForce(p0, p1, rotmat1, mtl, ff[1]);
	m_shearelm[2].computeNodalForce(p2, p3, rotmat2, mtl, ff[2]);
	m_shearelm[3].computeNodalForce(p2, p3, rotmat3, mtl, ff[3]);

	//accumulate forces on the nodes
	F[0] = ff[0]+ff[1];
	F[1] = -F[0];
	F[2] = ff[2]+ff[3];
	F[3] = -F[2];
}


static int test_func(void)
{
	CMeMaterialProperty mtl; 
	const int timerid = 0;
	const int N=20;
	CTetra2Element shell;
	Vector3d p0(0, -1, 0);
	Vector3d p1(0, 1, 0);
	Vector3d p2(0,0,1);
	Vector3d p3(1, 0, 0);
	Vector3d F[4];
	int quad[4]={0,1,2,3};
	shell.init(quad, p0, p1, p2, p3);

	startFastTimer(timerid);
	for (int i=0; i<N; i++){
		double x = (double)i/N;
		double y = sqrt(1.0-x*x);
		p2 = Vector3d(x, 0, y);
		//shell.computeForce(p0, p1, p2, p3, mtl, F);
		shell.updateState();
	}
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Shell elm run time:");

	return 1;
}

static int ntest = test_func();