//FILE: element_tetfem2.cpp
//Green strain tet element

#include <geomath.h>
#include "element_tetfem2.h"


void CTetFem2Element::_initElement(
	const CMeMaterialProperty& mtl,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
{
	//compute volume for each tet
	m_vol = fabs(computeTetrahedronVolume(p0, p1, p2, p3));

	//precompute the inverse of X
	Vector3d q1 = p1-p0;
	Vector3d q2 = p2-p0;
	Vector3d q3 = p3-p0;
	CopyVectorsToColumns(q1, q2, q3, m_matInvX);
	m_matInvX.Invert();
}


void CTetFem2Element::computeForce(
	const int __timeid, const int __isStatic,		//not used in this func.
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const CMeMaterialProperty &mtl, 
	Vector3d Force[4], double3x3* jac[4])
{
	//compute deformation gradient
	const Vector3d q1 = p1 - p0;
	const Vector3d q2 = p2 - p0;
	const Vector3d q3 = p3 - p0;
	double3x3 _x;		
	CopyVectorsToColumns(q1, q2, q3, _x);
	const double3x3 F = _x*m_matInvX;

	//green strain *2
	double3x3 strain = FtF(F);  strain.MinusIdentity();
	
	//stress
	double3x3 stress;	
	double lambda, miu;
	mtl.getLameCofficients(lambda, miu);
	const double vol2 = this->m_vol*(-0.5);
	lambda*=vol2;
	miu*=vol2;
	CMeMaterialProperty::IsotropicMaterialStress(lambda, miu, strain, stress);

	//compute the deriv of strain
	const double3x3 FT = TransposeMatrix(F);
	double3x3 deriv;
	Vector3d *drow0 = (Vector3d*)&deriv.x[0];
	Vector3d *drow1 = (Vector3d*)&deriv.x[3];
	Vector3d *drow2 = (Vector3d*)&deriv.x[6];
	const Vector3d *srow0 = (Vector3d*)&m_matInvX.x[0];
	const Vector3d *srow1 = (Vector3d*)&m_matInvX.x[3];
	const Vector3d *srow2 = (Vector3d*)&m_matInvX.x[6];
	*drow0 = *drow1 = *drow2 = *srow0;
	double3x3 dF1x = MultRow0(FT, deriv); //dF1x.AddTranspose();
	double3x3 dF1y = MultRow1(FT, deriv); //dF1y.AddTranspose();
	double3x3 dF1z = MultRow2(FT, deriv); //dF1z.AddTranspose();
	*drow0 = *drow1 = *drow2 = *srow1;
	double3x3 dF2x = MultRow0(FT, deriv); //dF2x.AddTranspose();
	double3x3 dF2y = MultRow1(FT, deriv); //dF2y.AddTranspose();
	double3x3 dF2z = MultRow2(FT, deriv); //dF2z.AddTranspose();
	*drow0 = *drow1 = *drow2 = *srow2;
	double3x3 dF3x = MultRow0(FT, deriv); //dF3x.AddTranspose();
	double3x3 dF3y = MultRow1(FT, deriv); //dF3y.AddTranspose();
	double3x3 dF3z = MultRow2(FT, deriv); //dF3z.AddTranspose();

	Force[1].x = InnerProd(dF1x, stress);
	Force[1].y = InnerProd(dF1y, stress);
	Force[1].z = InnerProd(dF1z, stress);
	Force[2].x = InnerProd(dF2x, stress);
	Force[2].y = InnerProd(dF2y, stress);
	Force[2].z = InnerProd(dF2z, stress);
	Force[3].x = InnerProd(dF3x, stress);
	Force[3].y = InnerProd(dF3y, stress);
	Force[3].z = InnerProd(dF3z, stress);
	Force[0] = -(Force[1]+Force[2]+Force[3]);

	//rotate back into world
	//if (jac[0]){//rotate the jacobian			
		//	double3x3 k;
		//	R = RT; R.Transpose();
		//}
	//F[0]=-(F[1]+F[2]+F[3]);
}


