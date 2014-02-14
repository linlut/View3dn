//FILE: thinsehll_element.cpp
#include <geomath.h>
#include "thinshell3_element.h"
//#include "simu_entity.h"
#include "save_stiffness.h"


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
inline void getReferencePlanesForTrianglePair(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	Vector3d &preX, Vector3d &preZ,
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& mat, double &xlen)
{
	Vector3d &Z0 = facenorm0;
	Vector3d &Z1 = facenorm1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	//compute X-axis
	X = q1; xlen = Magnitude(X); X /= xlen;
	const double dotXX = DotProd(X, preX);
	const double COS_ANGLETOL=0; //-0.707; //cos(3/4 PI)
	//if (dotXX<COS_ANGLETOL){
	//	printf("Flip X !!!\n");
	//	X=-X; xlen=-xlen;
	//}
	preX = X;
	//compute Z-axis
	const Vector3d& Y0 = q2;
	Z0 = CrossProd(X, Y0); Z0.normalize();
	const Vector3d& Y1 = q3;	
	Z1 = CrossProd(Y1, X); Z1.normalize();
	//compute average Z as the mean of Z0 and Z1
	Z = Z0+Z1; Z.normalize();
	const double dotZZ = DotProd(Z, preZ);
	//if (dotZZ<COS_ANGLETOL){
	//	printf("Flip Z!\n");
	//	Z=-Z;
	//}
	preZ = Z;
	//compute Y-axis
	Y = CrossProd(Z, X); 
}

//use the center plane of the dihedral angle as the reference plane
inline void getReferencePlanesForQuadPair(
	const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& mat, double &xlen)
{
	Vector3d Y0, Y1;
	Vector3d &Z0 = facenorm0;
	Vector3d &Z1 = facenorm1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	const Vector3d p23 = (p2+p3)*0.5;	//quad center
	const Vector3d p45 = (p4+p5)*0.5;	//quad center
	//X axis
	X = p1; xlen = Magnitude(X); X /= xlen;	
	//Z axis
	Y0 = p23;
	Z0 = CrossProd(X, Y0); 
	Y1 = p45;	
	Z1 = CrossProd(Y1, X); 
	Z0.normalize(); Z1.normalize();
	Z = Z0+Z1; Z.normalize();
	//Y axis
	Y = CrossProd(Z, X); 
}


void CThinshellElementFEM::_initElements(
	const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const double& thickness)
{
	tri0.init(1, mtl, p0, p1, p2, p3, thickness);
}


void CThinshellElementFEM::_initElements(
	const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	const double& thickness)
{
	double3x3 rot;
	Vector3d N0, N1;
	double len0;
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0, q4=p4-p0, q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, len0);
	tri0.init(rot, mtl, p0, p1, p2, p3, thickness);
	tri1.init(rot, mtl, p0, p1, p4, p5, thickness);
	//for faster computation of the force on node 1, we add the factors together
	tri0.m_F1a+=tri0.m_F1b+tri1.m_F1a+tri1.m_F1b;
}


void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[6],					//force in global coord.
	double3x3 *ppJacobian[13])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double xlen;
	double3x3 rot;
	Vector3d N0, N1;
	Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0, q4=p4-p0, q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, xlen);
	//load stiffness
	const double *s0 = tri0.getStiffness();
	const double *s1 = tri1.getStiffness();
	const double &A11 = s0[0]; const double &A12 = s0[1]; const double &A22 = s0[2];
	const double &B11 = s0[3]; const double &B12 = s0[4]; const double &B22 = s0[5];
	const double &C11 = s1[0]; const double &C12 = s1[1]; const double &C22 = s1[2];
	const double &D11 = s1[3]; const double &D12 = s1[4]; const double &D22 = s1[5];
	//force computation
	FF[1] = (A11+B11+C11+D11)*q1 + (A12*q2+B12*q3+C12*q4+D12*q5);
	FF[1] -= (tri0.m_F1a)*rot;	//we have added up +tri0.m_F1b+tri1.m_F1a+tri1.m_F1b
	FF[2] = (A12*q1+A22*q2) - tri0.m_F2*rot;	
	FF[3] = (B12*q1+B22*q3) - tri0.m_F3*rot;
	FF[4] = (C12*q1+C22*q4) - tri1.m_F2*rot;
	FF[5] = (D12*q1+D22*q5) - tri1.m_F3*rot;
	//check strain rate damping
	const double kd = mtl.getEffectiveYoungDamp();
	if (kd > 0.0){
		const Vector3d fd1=FF[1], fd2=FF[2], fd3=FF[3], fd4=FF[4], fd5=FF[5];
		FF[1]+=kd*(fd1-tri0.m_F1tn); 
		FF[2]+=kd*(fd2-tri0.m_F2tn); 
		FF[3]+=kd*(fd3-tri0.m_F3tn);
		FF[4]+=kd*(fd4-tri1.m_F2tn); 
		FF[5]+=kd*(fd5-tri1.m_F3tn); 
		tri0.m_F1tn=fd1, tri0.m_F2tn=fd2, tri0.m_F3tn=fd3, tri1.m_F2tn=fd4, tri1.m_F3tn=fd5;
	}
	//compute node 0 force
	FF[0] = -(FF[1]+FF[2]+FF[3]+FF[4]+FF[5]);
	//=================================================
	if (ppJacobian[0]){
		int i;
		const double kd1 = 1.0 + kd;
		for (i=0; i<6; i++) ppJacobian[i]->x[0] = s0[i]*kd1;
		for (i=0; i<6; i++) ppJacobian[i+6]->x[0] = s1[i]*kd1;
		//add spring stiffness
		ppJacobian[12]->setZeroMatrix();
		double *x = ppJacobian[12]->x; 
		x[0]= x[4] = x[8] = 0; //K;
	}
}


void CThinshellElementFEM::saveStiffness(CSparseMatrix33 *p, const double3x3 pjac[])
{
	double jac[13];
	const int i0 = m_nNodeID[0]; const int i1 = m_nNodeID[1];
	const int i2 = m_nNodeID[2]; const int i3 = m_nNodeID[3];
	const int i4 = m_nNodeID[4]; const int i5 = m_nNodeID[5];
	if (i5!=-1){//quad shell
		for (int i=0; i<13; i++) jac[i]=pjac[i].x[0];
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i2, &jac[0]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i3, &jac[3]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i4, &jac[6]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i5, &jac[9]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i1, i0, jac[12]);
	}
	else{//triangle shell
		for (int i=0; i<7; i++) jac[i]=pjac[i].x[0];
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i2, &jac[0]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i0, i1, i3, &jac[3]);
		SaveVertSitffMatrixIntoSparseMatrix(p, i1, i0, jac[6]);
	}
}



/*
void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[6],					//force in global coord.
	double3x3 *ppJacobian[13])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double3x3 rot;
	Vector3d N0, N1;
	double xlen;
	Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0, q4=p4-p0, q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, xlen);
	const Vector3d u1(xlen-tri0.m_len0, 0, 0);
	const Vector3d u2 = rot*q2 - tri0.m_qq2;
	const Vector3d u3 = rot*q3 - tri0.m_qq3;
	const Vector3d u4 = rot*q4 - tri1.m_qq2;
	const Vector3d u5 = rot*q5 - tri1.m_qq3;
	//force comp.
	const double *s0 = tri0.getStiffness();
	const double *s1 = tri1.getStiffness();
	const double &A11 = s0[0]; const double &A12 = s0[1]; const double &A22 = s0[2];
	const double &B11 = s0[3]; const double &B12 = s0[4]; const double &B22 = s0[5];
	const double &C11 = s1[0]; const double &C12 = s1[1]; const double &C22 = s1[2];
	const double &D11 = s1[3]; const double &D12 = s1[4]; const double &D22 = s1[5];
	FF[1] = (A11+B11+C11+D11)*u1 + A12*u2 + B12*u3 + C12*u4 + D12*u5;
	FF[2] = A12*u1 + A22*u2; 
	FF[3] = B12*u1 + B22*u3; 
	FF[4] = C12*u1 + C22*u4; 
	FF[5] = D12*u1 + D22*u5; 
	//spring force
	//const double E = mtl.getYoung()*m_thickness;
	//const double K= -2.0*E*(m_area0+m_area1)/(m_len0*m_len0);
	//const double springf = K*u1.x;
	//FF[1].x+=springf;
	//get world force
	FF[1] = FF[1]*rot;
	FF[2] = FF[2]*rot;
	FF[3] = FF[3]*rot;
	FF[4] = FF[4]*rot;
	FF[5] = FF[5]*rot;
	FF[0] = -(FF[1]+FF[2]+FF[3]+FF[4]+FF[5]);
	//=================================================
	if (ppJacobian[0]){
		int i;
		for (i=0; i<6; i++) ppJacobian[i]->x[0] = s0[i];
		for (i=0; i<6; i++) ppJacobian[i+6]->x[0] = s1[i];
		//add spring stiffness
		ppJacobian[12]->ZeroMatrix();
		double *x = ppJacobian[12]->x; 
		x[0]= x[4] = x[8] = 0; //K;
	}
}

static int test2()
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1);
	const int isstatic=0;
	const int pNodeIDBuffer[6]={0, 1, 2, 3, 4, 5};
	const double x=1;
	Vector3d p[6]={
		Vector3d(0, 0, 0), Vector3d(x, 0, 0),
		Vector3d(x, 1, 0), Vector3d(0, 1, 0),
		Vector3d(0, -1, 0), Vector3d(x, -1, 0)
	};

	const double thickness=1;
	CThinshellElementFEM e(mtl, isstatic, pNodeIDBuffer, p[0], p[1], p[2], p[3], p[4], p[5], thickness);
	Vector3d F[6];
	double3x3 *ppJacobian[13]={NULL};
	const double r = sqrt(2.0)/2;
	const double d = 0.0e-2;
	p[2]=Vector3d(x, r, r-d);
	p[3]=Vector3d(0, r, r+d);
	p[4]=Vector3d(0, -r, r+d);
	p[5]=Vector3d(x, -r, r-d);
	e.computeForce(p[0], p[1], p[2], p[3], p[4], p[5], 0, mtl, F, ppJacobian);
	return 1;
}

static int test1()
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1);
	const int isstatic=0;
	const int pNodeIDBuffer[6]={0, 1, 2, 3, 4, 5};
	const double x=1;
	Vector3d p[4]={
		Vector3d(0, 0, 0), Vector3d(x, 0, 0),
		Vector3d(x/2, 1, 0), Vector3d(x/2, -1, 0)
	};
	const double thickness=1;
	CThinshellElementFEM e(mtl, isstatic, pNodeIDBuffer, p[0], p[1], p[2], p[3], thickness);
	Vector3d F[4];
	double3x3 *ppJacobian[13]={NULL};
	const double r = sqrt(2.0)/2;
	const double d = 0e-2;
	p[2]=Vector3d(x/2, r, (r-d));
	p[3]=Vector3d(x/2, -r, (r+d));
	e.computeForce(p[0], p[1], p[2], p[3], 0, mtl, F, ppJacobian);
	return 1;
}

//static int r=test2();

*/