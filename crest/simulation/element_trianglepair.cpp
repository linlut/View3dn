//FILE: element_tetfem3.cpp
#include <geomath.h>
#include "element_trianglepair.h"


inline void computeStiffnessMat(const double3x3& strainDeriv, const double& K, double3x3 *ppJacobian[3]) 
{
	const Vector3d *a = (const Vector3d *)&strainDeriv.x[0];
	const Vector3d *b = (const Vector3d *)&strainDeriv.x[3];
	const Vector3d *c = (const Vector3d *)&strainDeriv.x[6];
	const double aa = DotProd(*a, *a);
	const double ab = DotProd(*a, *b);
	const double ac = DotProd(*a, *c);
	const double bb = DotProd(*b, *b);
	const double bc = DotProd(*b, *c);
	const double cc = DotProd(*c, *c);
	double S11 = aa*K;
	const double S12 = (ab+ac)*K;
	double S22 = (bb+cc+bc+bc)*K;

	ppJacobian[0]->setZeroMatrix();
	ppJacobian[1]->setZeroMatrix();
	ppJacobian[2]->setZeroMatrix();
	double *x;
	x=ppJacobian[0]->x; x[0]=x[4]=x[8]=S11;
	x=ppJacobian[1]->x; x[0]=x[4]=x[8]=S12;
	x=ppJacobian[2]->x; x[0]=x[4]=x[8]=S22;
}


CTriangle3DPairElement::CTriangle3DPairElement()
{
	m_XInv0.setZeroMatrix();
	m_XInv1.setZeroMatrix();
	m_vol0 = m_vol1 = m_len0 = 0;
	m_stiffness[0] = m_stiffness[1] = Vector3d(0);
}


double CTriangle3DPairElement::getStiffnessRatio(void) const
{
	const double a0 = fabs(m_stiffness[0].x) + fabs(m_stiffness[0].z);
	const double b0 = fabs(m_stiffness[0].y) + 1e-20;
	const double K0 = a0/b0;
	const double a1 = fabs(m_stiffness[1].x) + fabs(m_stiffness[1].z);
	const double b1 = fabs(m_stiffness[1].y) + 1e-20;
	const double K1 = a1/b1;
	return K0+K1;
}


void CTriangle3DPairElement::_initElement(
	const CMeMaterialProperty &mtl,
	const double3x3& rot0, const double3x3& rot1, 
	const Vector3d& N0, const Vector3d& N1, const double &len0,
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3)
{
	const double HEIGHT_DELTA = (20.0/81920);
	//compute rotated vertices
	m_len0 = len0;
	const Vector3d qq1(m_len0, 0, 0);
	const Vector3d qq2 = rot0*q2;
	const Vector3d qq3 = rot1*q3;
	const double DELTA = m_len0*HEIGHT_DELTA;
	const Vector3d qq4 = rot0*(q2+DELTA*N0);
	const Vector3d qq5 = rot1*(q3+DELTA*N1);
	//derivitive pF/pX
	CopyVectorsToColumns(qq1, qq2, qq4, m_XInv0);
	CopyVectorsToColumns(qq1, qq3, qq5, m_XInv1);
	m_XInv0.Invert();
	m_XInv1.Invert();
	//stiffness related comp.
	const int NSTIFF=6;
	double3x3 jac[NSTIFF];
	double3x3 *ppJacobian[NSTIFF]={jac, jac+1, jac+2, jac+3, jac+4, jac+5}; 
	const double E = -mtl.getYoung();
	computeStiffnessMat(m_XInv0, E*m_vol0, ppJacobian);
	computeStiffnessMat(m_XInv1, E*m_vol1, ppJacobian+NSTIFF/2);
	double *pstiff = &m_stiffness[0].x;
	for (int i=0; i<NSTIFF; i++) pstiff[i]=jac[i].x[0];
	//Static force computation: a faster comp. method
	GET_STIFF_VARS(m_stiffness);
	m_F1a = A11*qq1 + A12*qq2;
	m_F2  = A12*qq1 + A22*qq2;
	m_F1b = B11*qq1 + B12*qq3;
	m_F3  = B12*qq1 + B22*qq3;
	//for strain rate damping
	m_F1tn = m_F2tn = m_F3tn = Vector3d(0);
}


void CTriangle3DPairElement::init(
	const int rottype, const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const double& thickness)
{
	//compute volume for each triangle, each truss edge
	m_vol0 = fabs(triangle_area(p0, p1, p2))*thickness;
	m_vol1 = fabs(triangle_area(p0, p1, p3))*thickness;
	//construct the world/local transform matrix for the two triangles
	Vector3d N0, N1;
	double3x3 rot0, rot1;
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	ASSERT0(rottype==0 || rottype==1);
	if (rottype==0)
		getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot0, rot1, m_len0);
	else{
		getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot0, m_len0);
		rot1 = rot0;
	}
	_initElement(mtl, rot0, rot1, N0, N1, m_len0, q1, q2, q3);
}


void CTriangle3DPairElement::init(
	const double3x3& rot, const CMeMaterialProperty &mtl,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const double& thickness)
{
	//compute volume for each triangle, each truss edge
	m_vol0 = fabs(triangle_area(p0, p1, p2))*thickness;
	m_vol1 = fabs(triangle_area(p0, p1, p3))*thickness;
	m_len0 = Distance(p0, p1);
	//construct the world/local transform matrix for the two triangles
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	Vector3d N0 = CrossProd(q1, q2); N0.normalize();
	Vector3d N1 = CrossProd(q3, q1); N1.normalize();
	_initElement(mtl, rot, rot, N0, N1, m_len0, q1, q2, q3);
}


 /*
 	//compute elastic+bending force for a shear element in triangle mesh
	//if jacobian is not needed, fill the array with 4 NULL pointers
	void computeForce(
		const double& xlen, const Vector3d &q1, const Vector3d &q2, const Vector3d &q3,
		const double3x3& rot0, const double3x3& rot1,
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d FF[4])					//force in global coord.
	{
		GET_STIFF_VARS(m_stiffness);
		const double u1x = xlen-m_len0;	
		const Vector3d u2 = rot0*q2 - m_qq2;
		const Vector3d u3 = rot1*q3 - m_qq3;
		FF[0] = A12*u2, FF[0].x += A11*u1x;
		FF[1] = B12*u3, FF[1].x += B11*u1x;
		FF[2] = A22*u2, FF[2].x += A12*u1x,
		FF[3] = B22*u3, FF[3].x += B12*u1x; 
		//get world force
		FF[1] = FF[0]*rot0 + FF[1]*rot1;
		FF[2] = FF[2]*rot0;
		FF[3] = FF[3]*rot1;
		FF[0] = -(FF[1]+FF[2]+FF[3]);
	}
	
void CTriangle3DPairElement::computeForce(
		const double& xlen, const Vector3d &q1, const Vector3d &q2, const Vector3d &q3,
		const double3x3& rot,
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d FF[4]); 				//force in global coord.
	{
		GET_STIFF_VARS(m_stiffness);
		const double u1x = xlen-m_len0;
		const Vector3d u2 = rot*q2 - m_qq2;
		const Vector3d u3 = rot*q3 - m_qq3;
		FF[1] = A12*u2 + B12*u3; FF[1].x += (A11+B11)*u1x;
		FF[2] = A22*u2; FF[2].x += A12*u1x; 
		FF[3] = B22*u3; FF[3].x += B12*u1x; 
		//get world force
		FF[1] = FF[1]*rot;
		FF[2] = FF[2]*rot;
		FF[3] = FF[3]*rot;
		FF[0] = -(FF[1]+FF[2]+FF[3]);
	}

	void computeForce(
		const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
		const CMeMaterialProperty &mtl, //material parameters
		Vector3d F[4])					//force in global coord.
	{
		//generate the two world/local transform matrices
		const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
		double3x3 rot0, rot1;
		Vector3d N0, N1;
		double xlen;
		getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot0, rot1, xlen);
		//then, call the static force comp. function
		computeForce(xlen, q1, q2, q3, rot0, rot1, mtl, F);
	}
	*/
