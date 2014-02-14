//FILE: thinsehll_element.cpp
#include <geomath.h>
#include <ztime.h>
#include "thinshell_element.h"


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
/*
void GenReferencePlanesForTrianglePair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	double3x3& mat, double3x3& mat2)
{
	//------------PLANE0--------------------------
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	X = p1 - p0; X.Normalize();
	Y = p2 - p0;
	Z = CrossProd(X, Y); Z.Normalize();
	Y = CrossProd(Z, X); 

	//------------PLANE1---------------------------
	Vector3d &X2 = *((Vector3d*)(&mat2.x[0]));
	Vector3d &Y2 = *((Vector3d*)(&mat2.x[3]));
	Vector3d &Z2 = *((Vector3d*)(&mat2.x[6]));
	X2 = X;
	Y2 = p0 - p3; 
	Z2 = CrossProd(X2, Y2); Z2.Normalize();
	Y2 = CrossProd(Z2, X2);
}
*/
void GenReferencePlanesForTrianglePair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	double3x3& mat, double3x3& mat2)
{
	Vector3d Y0, Y1, Z0, Z1;
	//------------PLANE0--------------------------
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	X = p1 - p0; X.Normalize();
	Y0 = p2 - p0;
	Z0 = CrossProd(X, Y0); 
	Y1 = p3 - p0;	
	Z1 = CrossProd(Y1, X); 
	Z0.Normalize();
	Z1.Normalize();
	Z = Z0+Z1; Z.Normalize();
	Y = CrossProd(Z, X); 
	//------------PLANE1---------------------------
	mat2 = mat;
}



//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
/*
void GenReferencePlanesForQuadPair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	double3x3& mat, double3x3& mat2)
{
	//------------PLANE0--------------------------
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	const Vector3d p01 = (p0+p1);
	const Vector3d p23 = (p2+p3+p01)*0.25;
	const Vector3d p45 = (p4+p5+p01)*0.25;
	X = p1 - p0; X.Normalize();
	Y = p23 - p0;
	Z = CrossProd(X, Y); Z.Normalize();
	Y = CrossProd(Z, X); 

	//------------PLANE1---------------------------
	Vector3d &X2 = *((Vector3d*)(&mat2.x[0]));
	Vector3d &Y2 = *((Vector3d*)(&mat2.x[3]));
	Vector3d &Z2 = *((Vector3d*)(&mat2.x[6]));
	X2 = X;
	Y2 = p0 - p45; 
	Z2 = CrossProd(X2, Y2); Z2.Normalize();
	Y2 = CrossProd(Z2, X2); 
}
*/

//use the center plane of the dihedral angle as the reference plane
void GenReferencePlanesForQuadPair(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	double3x3& mat, double3x3& mat2)
{
	Vector3d Y0, Y1, Z0, Z1;
	//------------PLANE0--------------------------
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	const Vector3d p01 = (p0+p1);
	const Vector3d p23 = (p2+p3+p01)*0.25;
	const Vector3d p45 = (p4+p5+p01)*0.25;
	X = p1 - p0; X.Normalize();
	Y0 = p23 - p0;
	Z0 = CrossProd(X, Y0); 
	Y1 = p45 - p0;	
	Z1 = CrossProd(Y1, X); 
	Z0.Normalize();
	Z1.Normalize();
	Z = Z0+Z1; Z.Normalize();
	Y = CrossProd(Z, X); 
	//------------PLANE1---------------------------
	mat2 = mat;
}



static void _printAllVertices(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5)
{
	Vector3d v;
	v = p0; printf("0: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p1; printf("1: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p2; printf("2: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p3; printf("3: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p4; printf("4: %10lg %10lg %10lg\n", v.x, v.y, v.z);
	v = p5; printf("5: %10lg %10lg %10lg\n", v.x, v.y, v.z);
}

//===========================================================================
/*
static int test_func(void)
{
    CMeMaterialProperty mtl;
    const int timerid = 0;
    const int N=1000000;
    CThinshellElement shell;
    Vector3d p0(0, -1, 0);
    Vector3d p1(0, 1, 0);
    Vector3d p2(0,0,1);
    Vector3d p3(1, 0, 0);
    Vector3d F[4];
    double3x3 *ppJacobian[4]={NULL, NULL, NULL, NULL};
    double t=0.01;
    int quad[4]={0,1,2,3};
    shell.init(0, quad, p0, p1, p2, p3, t);

    startFastTimer(timerid);
    const int j=3;
    for (int i=0; i<N; i++){
        const double k = 1.0*i/N;
        double a = 180.0*k;
        double t = PI*k;
        //double z = cos(t);
        //double x = sin(t);
        double z = 1;
        double x = 0;
        p2 = Vector3d(x, 0, z);
        shell.computeForce(p0, p1, p2, p3, 0, mtl, F, ppJacobian);
        //printf("Angel %3lg, Force is %8lg %8lg %8lg\n", a, F[j].x, F[j].y, F[j].z);
    }
    stopFastTimer(timerid);
    reportTimeDifference(timerid, "Shell template elm run time:");

    return 1;
}

*/
void test_thinshellelm(void * pshell, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d&p2, const Vector3d&p3, const Vector3d&p4, const Vector3d&p5, 
	CMeMaterialProperty&mtl)
{
	CThinshellElement *tshell = (CThinshellElement *)pshell;
    Vector3d F[6];
    double3x3 *ppJacobian[6]={NULL, NULL, NULL, NULL, NULL, NULL};
    const int timerid = 0;
    const int N=100000;
    startFastTimer(timerid);
    for (int i=0; i<N; i++){
		tshell->computeForce(p0, p1, p2, p3, p4, p5, 0, mtl, F, ppJacobian);
    }
    stopFastTimer(timerid);
    const double t = getTimeDifference(timerid);
	printf("Shell Template Element run time:%lg, Looping %d\n", t, N);
}

void computeForceNoTemp(void *shell, 
		const Vector3d &p0, const Vector3d &p1, const Vector3d&p2, const Vector3d&p3, const Vector3d&p4, const Vector3d&p5, 
		const int isstatic,
		CMeMaterialProperty&mtl, 
		Vector3d *force, 
		double3x3 *ppjac[6])
{
	CThinshellElement *tshell = (CThinshellElement *)shell;
	tshell->computeForce(p0, p1, p2, p3, p4, p5, isstatic, mtl, force, ppjac);
}


static int test_func(void)
{
	int i;
    CMeMaterialProperty mtl;
    CThinshellElement shell[26];
    Vector3d p0(0, 0, 0);
    Vector3d p1(0,1,0);
    Vector3d p2(-1, 1, 0);
    Vector3d p3(-1, 0, 0);
    Vector3d p4(1, 0, 0);
    Vector3d p5(1, 1, 0);
    double t=0.01;
    int quad[6]={0,1,2,3, 4, 5};
	for (i=0; i<26; i++)
		shell[i].init(0, quad, p0, p1, p2, p3, p4, p5, t);
	test_thinshellelm(&shell[0], p0, p1, p2, p3, p4, p5, mtl);
    return 1;
}


static int ntest = test_func();

