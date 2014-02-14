//FILE: element_tetfem3.cpp
#include <geomath.h>
#include "element_tetfem3.h"
#include "save_stiffness.h"


void CTetFem3Element::_initElement(
	const CMeMaterialProperty& mtl, const double &thickness,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
{
	tri0.init(0, mtl, p0, p1, p2, p3, thickness);
	tri1.init(0, mtl, p2, p3, p0, p1, thickness);

	m_bFlipProcessed = false;
	m_normal[0] = compute_triangle_normal(p0, p1, p2);
	m_normal[1] = compute_triangle_normal(p0, p3, p1);
	m_normal[2] = compute_triangle_normal(p2, p3, p0);
	m_normal[3] = compute_triangle_normal(p2, p1, p3);
}


void CTetFem3Element::init(
	const CMeMaterialProperty& mtl, const int *nodeid, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
{
	const double signedVol = DotProd(p3-p0, CrossProd(p1-p0, p2-p0))/6.0;
	const bool positiveFlag = (signedVol>0)? true: false;
	if (!positiveFlag){ //negative volume, reassign the node indices
		int ibuf[4]={nodeid[0], nodeid[1], nodeid[2], nodeid[3]};
		ibuf[1] = nodeid[2], ibuf[2] = nodeid[1];
		init(mtl, ibuf, p0, p2, p1, p3);
		return;
	}

	const double tvol = fabs(signedVol);
	const double a0 = fabs(triangle_area(p0, p1, p2));
	const double a1 = fabs(triangle_area(p0, p1, p3));
	const double a2 = fabs(triangle_area(p0, p2, p3));
	const double a3 = fabs(triangle_area(p1, p2, p3));
	const double th = tvol/(a0+a1+a2+a3);
	CTetFem3Element e[6];
	double r[6];
	e[0]._initElement(mtl, th, p0, p1, p2, p3);
	e[1]._initElement(mtl, th, p0, p2, p3, p1);
	e[2]._initElement(mtl, th, p0, p3, p1, p2);
	e[3]._initElement(mtl, th, p1, p0, p3, p2);
	e[4]._initElement(mtl, th, p2, p0, p1, p3);
	e[5]._initElement(mtl, th, p3, p0, p2, p1);
	r[0] = e[0].getStiffnessRatio();
	r[1] = e[1].getStiffnessRatio();
	r[2] = e[2].getStiffnessRatio();
	r[3] = e[3].getStiffnessRatio();
	r[4] = e[4].getStiffnessRatio();
	r[5] = e[5].getStiffnessRatio();
	const double rmin0 = _MIN3_(r[0], r[1], r[2]);
	const double rmin1 = _MIN3_(r[3], r[4], r[5]);
	const double rmin = _MIN_(rmin0, rmin1);
	if (rmin>1e3)
		printf("Still not solved!, rmin=%lf\n", rmin); 
	if (r[0]==rmin)
		*this=e[0], m_nodeID[0]=nodeid[0], m_nodeID[1]=nodeid[1], m_nodeID[2]=nodeid[2], m_nodeID[3]=nodeid[3];
	else if (r[1]==rmin)
		*this=e[1], m_nodeID[0]=nodeid[0], m_nodeID[1]=nodeid[2], m_nodeID[2]=nodeid[3], m_nodeID[3]=nodeid[1];
	else if (r[2]==rmin)
		*this=e[2], m_nodeID[0]=nodeid[0], m_nodeID[1]=nodeid[3], m_nodeID[2]=nodeid[1], m_nodeID[3]=nodeid[2];
	else if (r[3]==rmin)
		*this=e[3], m_nodeID[0]=nodeid[1], m_nodeID[1]=nodeid[0], m_nodeID[2]=nodeid[3], m_nodeID[3]=nodeid[2];
	else if (r[4]==rmin)
		*this=e[4], m_nodeID[0]=nodeid[2], m_nodeID[1]=nodeid[0], m_nodeID[2]=nodeid[1], m_nodeID[3]=nodeid[3];
	else 
		*this=e[5], m_nodeID[0]=nodeid[3], m_nodeID[1]=nodeid[0], m_nodeID[2]=nodeid[2], m_nodeID[3]=nodeid[1];
}


inline void getReferencePlanesForTet(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	double3x3& rot0, double3x3& rot1, double &xlen)
{
	Vector3d &X0 = *((Vector3d*)(&rot1.x[0]));
	Vector3d &Y0 = *((Vector3d*)(&rot1.x[3]));
	Vector3d &Z0 = *((Vector3d*)(&rot1.x[6]));
	Vector3d &X1 = *((Vector3d*)(&rot0.x[0]));
	Vector3d &Y1 = *((Vector3d*)(&rot0.x[3]));
	Vector3d &Z1 = *((Vector3d*)(&rot0.x[6]));
	//set X axis as the vector q1
	xlen = Magnitude(q1);
	X0 = X1 = q1/xlen; 
	//compute Z axis
	Y0 = q2; Z0 = CrossProd(X0, Y0); Z0.normalize();
	Y1 = q3; Z1 = CrossProd(Y1, X1); Z1.normalize();
	//then compute Y axis
	Y0 = CrossProd(Z0, X0); 
	Y1 = CrossProd(Z1, X1); 
}


/*
void CTetFem3Element::computeForce(
	const int __timeid, const int __isstatic,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const CMeMaterialProperty &mtl, 
	Vector3d F[4], double3x3* jac[12])
{
	double3x3 rot00, rot01, rot10, rot11;
	Vector3d F0[4], F1[4];
	double xlen0, xlen1;
	//tri pair 0
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	getReferencePlanesForTet(q1, q2, q3, rot00, rot01, xlen0);
	tri0.computeForce(xlen0, q1, q2, q3, rot00, rot01, mtl, F0);
	//tri pair 1
	const Vector3d s1=p3-p2, s2=-q2, s3=p1-p2;
	getReferencePlanesForTet(s1, s2, s3, rot10, rot11, xlen1);
	tri1.computeForce(xlen1, s1, s2, s3, rot10, rot11, mtl, F1);
	//sum of the forces
	F[1] = F0[1] + F1[3];
	F[2] = F0[2] + F1[0];
	F[3] = F0[3] + F1[1];
	F[0] = F0[0] + F1[2];
}
*/


void CTetFem3Element::computeForce(
	const int __timeid, const int __isstatic,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const CMeMaterialProperty &mtl, 
	Vector3d F[4], double3x3* jac[12])
{
	double xlen0, xlen1;
	Vector3d F0[4], F1[4];
	double3x3 rot0, rot1, rot2, rot3;
	Vector3d &x0 = *((Vector3d*)(&rot1.x[0]));
	Vector3d &y0 = *((Vector3d*)(&rot1.x[3]));
	Vector3d &z0 = *((Vector3d*)(&rot1.x[6]));
	Vector3d &x1 = *((Vector3d*)(&rot0.x[0]));
	Vector3d &y1 = *((Vector3d*)(&rot0.x[3]));
	Vector3d &z1 = *((Vector3d*)(&rot0.x[6]));
	Vector3d &x2 = *((Vector3d*)(&rot3.x[0]));
	Vector3d &y2 = *((Vector3d*)(&rot3.x[3]));
	Vector3d &z2 = *((Vector3d*)(&rot3.x[6]));
	Vector3d &x3 = *((Vector3d*)(&rot2.x[0]));
	Vector3d &y3 = *((Vector3d*)(&rot2.x[3]));
	Vector3d &z3 = *((Vector3d*)(&rot2.x[6]));
	const Vector3d p10=p1-p0, p20=p2-p0, p30=p3-p0, p32=p3-p2, p02=-p20, p12=p1-p2;
	xlen0= Magnitude(p10); xlen1 = Magnitude(p32); 
	x0 = x1 = p10/xlen0; x2 = x3 = p32/xlen1;
	z0 = CrossProd(x0, p20); z0.normalize();
	z1 = CrossProd(p30, x1); z1.normalize();
	z2 = CrossProd(x2, p02); z2.normalize();
	z3 = CrossProd(p12, x3); z3.normalize();
	y0 = CrossProd(z0, x0);
	y1 = CrossProd(z1, x1);
	y2 = CrossProd(z2, x2);
	y3 = CrossProd(z3, x3);
	//*process inverted element
	const double tetvol = DotProd(z0, p30);
	if (tetvol<0){
		if (!m_bFlipProcessed){
			m_bFlipProcessed = true;
			const double NAGONE = -1.0;
			double d0 = DotProd(z0, m_normal[0]); d0 += NAGONE;
			double d1 = DotProd(z1, m_normal[1]); d1 += NAGONE;
			double d2 = DotProd(z2, m_normal[2]); d2 += NAGONE;
			double d3 = DotProd(z3, m_normal[3]); d3 += NAGONE;
			const double sum0= d0*d0+d1*d1, sum1= d2*d2+d3*d3;
			m_flipEdgeID=1;
			if (sum0>sum1) m_flipEdgeID=0;
		}
		//m_flipEdgeID = 1;
		if (m_flipEdgeID==0)
			xlen0=-xlen0, x0=-x0, z0=-z0, x1=-x1, z1=-z1;
		else
			xlen1=-xlen1, x2=-x2, z2=-z2, x3=-x3, z3=-z3;
	}
	else{
		m_bFlipProcessed = false;
		m_normal[0] = z0, m_normal[1] = z1;
		m_normal[2] = z2, m_normal[3] = z3;
	}
	//*/

	tri0.computeForce(xlen0, p10, p20, p30, rot0, rot1, mtl, F0);
	//F1[0]=F1[1]=F1[2]=F1[3]=Vector3d(0);
	tri1.computeForce(xlen1, p32, p02, p12, rot2, rot3, mtl, F1);
	//F0[0]=F0[1]=F0[2]=F0[3]=Vector3d(0);

	//sum of the forces
	F[1] = F0[1] + F1[3];
	F[2] = F0[2] + F1[0];
	F[3] = F0[3] + F1[1];
	F[0] = F0[0] + F1[2];

	//need to compute stiffness
	if (jac[0]){
		const double kd1 = mtl.getEffectiveYoungDamp()+1.0;
		const double * s = tri0.getStiffness();
		for (int i=0; i<6; i++) jac[i]->x[0] = s[i]*kd1;
		const double * t = tri1.getStiffness();
		for (int i=0; i<6; i++) jac[i+6]->x[0] = t[i]*kd1;
	}
}


void CTetFem3Element::saveStiffness(CSparseMatrix33 *p, const double3x3 jac[])
{
	const int *x = m_nodeID;
	double s[12];
	for (int i=0; i<12; i++) s[i] = jac[i].x[0];
	SaveVertSitffMatrixIntoSparseMatrix(p, x[0], x[1], x[2], s);
	SaveVertSitffMatrixIntoSparseMatrix(p, x[0], x[1], x[3], s+3);
	SaveVertSitffMatrixIntoSparseMatrix(p, x[2], x[3], x[0], s+6);
	SaveVertSitffMatrixIntoSparseMatrix(p, x[2], x[3], x[1], s+9);
}
