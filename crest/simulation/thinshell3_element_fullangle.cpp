//FILE: thinsehll_element.cpp
#include <geomath.h>
#include "thinshell3_element.h"
#include "simu_entity.h"

/*
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
	X = p1; 
	xlen = Magnitude(X);
	X /= xlen;	//normalize
	Y0 = p23;
	Z0 = CrossProd(X, Y0); 
	Y1 = p45;	
	Z1 = CrossProd(Y1, X); 
	Z0.Normalize();
	Z1.Normalize();
	Z = Z0+Z1; Z.Normalize();
	Y = CrossProd(Z, X); 
}


const double HEIGHT_DELTA = (1.0/4096.0);

void CThinshellElementFEM::_initElements(
	const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const double& thickness)
{
	//compute volume for each triangle, each truss edge
	m_area0 = fabs(triangle_area(p0, p1, p2));
	m_area1 = fabs(triangle_area(p0, p1, p3));
	m_thickness = thickness;

	//construct the world/local transform matrix for the two triangles
	Vector3d N0, N1;
	double3x3 rot0, rot1;
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot0, rot1, m_len0);
	const Vector3d qq1(m_len0, 0, 0);
	const Vector3d qq2 = rot0*q2;
	const Vector3d qq3 = rot1*q3;
	m_qq2 = qq2; m_qq3 = qq3;
	const double DELTA = m_len0*HEIGHT_DELTA;
	const Vector3d qq4 = rot0*(q2+DELTA*N0);
	const Vector3d qq5 = rot1*(q3+DELTA*N1);

	//derivitive pF/pX
	copyVectorsToColumns(qq1, qq2, qq4, m_XInv0);
	copyVectorsToColumns(qq1, qq3, qq5, m_XInv1);
	m_XInv0.Invert();
	m_XInv1.Invert();

	//stiffness related comp.
	const int NSTIFF=6;
	double3x3 jac[NSTIFF], *ppJacobian[NSTIFF]={jac, jac+1, jac+2, jac+3, jac+4, jac+5}; 
	const double E = mtl.getYoung()*m_thickness;
	computeStiffnessMat(m_XInv0, -E*m_area0, ppJacobian);
	computeStiffnessMat(m_XInv1, -E*m_area1, ppJacobian+3);
	double *pstiff = &m_stiffness[0].x;
	for (int i=0; i<NSTIFF; i++) pstiff[i]=jac[i].x[0];
}


void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[4],					//force in global coord.
	double3x3 *ppJacobian[7])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double3x3 rot0, rot1;
	Vector3d N0, N1, q1=p1-p0, q2=p2-p0, q3=p3-p0;
	double xlen;
	getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot0, rot1, xlen);
	const double u1x = xlen-m_len0;	//u1y=u1z=0
	const Vector3d u2 = rot0*q2 - m_qq2;
	const Vector3d u3 = rot1*q3 - m_qq3;
	//force comp.
	const double &A11 = m_stiffness[0].x;
	const double &A12 = m_stiffness[0].y;
	const double &A22 = m_stiffness[0].z;
	const double &B11 = m_stiffness[1].x;
	const double &B12 = m_stiffness[1].y;
	const double &B22 = m_stiffness[1].z;
	FF[0] = A12*u2, FF[0].x += A11*u1x;
	FF[1] = B12*u3, FF[1].x += B11*u1x;
	FF[2] = A22*u2, FF[2].x += A12*u1x,
	FF[3] = B22*u3, FF[3].x += B12*u1x; 
	//spring force
	const double E = mtl.getYoung()*m_thickness;
	const double K= -0.0*E*(m_area0+m_area1)/(m_len0*m_len0);
	const double springf = K*u1x;
	FF[1].x+=springf;
	//get world force
	FF[1] = FF[0]*rot0 + FF[1]*rot1;
	FF[2] = FF[2]*rot0;
	FF[3] = FF[3]*rot1;
	FF[0] = -(FF[1]+FF[2]+FF[3]);
	//=================================================
	if (ppJacobian[0]){
		ppJacobian[0]->x[0]=A11; ppJacobian[1]->x[0]=A12; ppJacobian[2]->x[0]=A22;
		ppJacobian[3]->x[0]=B11; ppJacobian[4]->x[0]=B12; ppJacobian[5]->x[0]=B22;
		//add spring stiffness
		double *x = ppJacobian[6]->x; 
		x[0]= x[4] = x[8] = K;
	}
}

*/