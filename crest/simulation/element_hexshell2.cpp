//FILE: element_tetfem3.cpp
#include <geomath.h>
#include "element_hexshell.h"
#include "save_stiffness.h"


//==================================================
//-0.5 -0.5 -0.5
//0.5 -0.5 -0.5
//-0.5 0.5 -0.5
//0.5 0.5 -0.5
//-0.5 -0.5 0.5
//0.5 -0.5 0.5
//-0.5 0.5 0.5
//0.5 0.5 0.5
//element node seq: 1 2 4 3 5 6 8 7
//==================================================
void CHexshellElement::init(
	const CMeMaterialProperty& mtl, const int *nodeid, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3,
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7)
{
	for (int i=0; i<8; i++) m_nodeID[i]=nodeid[i];
	//total volume
	Vector3d v[8] = {p0, p1, p2, p3, p4, p5, p6, p7};
	Vector8i hexelm(0,1,2,3,4,5,6,7);
	const double vol = computeHexhedronVolume(v, hexelm);
	//total area
	const double a0 = fabs(quad_area(p0, p1, p2, p3));
	const double a1 = fabs(quad_area(p0, p1, p5, p4));
	const double a2 = fabs(quad_area(p1, p2, p6, p5));
	const double a3 = fabs(quad_area(p2, p3, p7, p6));
	const double a4 = fabs(quad_area(p3, p0, p4, p7));
	const double a5 = fabs(quad_area(p4, p5, p6, p7));
	const double sumarea = a0+a1+a2+a3+a4+a5;
	//
	const double th = vol/sumarea;
	_initElement(mtl, th, p0, p1, p2, p3, p4, p5, p6, p7);
}


void CHexshellElement::_initElement(
	const CMeMaterialProperty& mtl, const double &thickness,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3,
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7)
{
	const int isstatic = 0;
	int *x = m_nodeID;
	int nodeid0[6] = {x[0], x[1], x[2], x[3], x[4], x[5]};
	int nodeid1[6] = {x[5], x[6], x[2], x[1], x[4], x[7]};
	int nodeid2[6] = {x[3], x[7], x[4], x[0], x[2], x[6]};
	qpair0.init(mtl, isstatic, nodeid0, p0, p1, p2, p3, p4, p5, thickness);
	qpair1.init(mtl, isstatic, nodeid1, p5, p6, p2, p1, p4, p7, thickness);
	qpair2.init(mtl, isstatic, nodeid2, p3, p7, p4, p0, p2, p6, thickness);
}


void CHexshellElement::computeForce(
	const int __timeid, const int __isstatic,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7,
	const CMeMaterialProperty &mtl, 
	Vector3d F[8], double3x3* jac[39])
{
	const int isStatic = 0;
	const int NN = 13;
	const int NN2 = NN+NN;
	Vector3d F0[6], F1[6], F2[6];
	qpair0.computeForce(p0, p1, p2, p3, p4, p5, isStatic, mtl, F0, jac);
	qpair1.computeForce(p5, p6, p2, p1, p4, p7, isStatic, mtl, F1, jac+NN);
	qpair2.computeForce(p3, p7, p4, p0, p2, p6, isStatic, mtl, F2, jac+NN2);
	F[0]=F0[0]+F2[3];
	F[1]=F0[1]+F1[3];
	F[2]=F0[2]+F1[2]+F2[4];
	F[3]=F0[3]+F2[0];
	F[4]=F0[4]+F1[4]+F2[2];
	F[5]=F0[5]+F1[0];
	F[6]=F1[1]+F2[5];
	F[7]=F1[5]+F2[1];
}


void CHexshellElement::saveStiffness(CSparseMatrix33 *p, const double3x3 jac[])
{
	const int N=13;
	const int NN=N+N;
	qpair0.saveStiffness(p, jac);
	qpair1.saveStiffness(p, &jac[N]);
	qpair2.saveStiffness(p, &jac[NN]);
}
