//FILE: element_tetfem.cpp
#include <geomath.h>
#include <decompose.h>			//Polar decomposition
#include "element_tetfem.h"
#include "element_shear.h"
#include "zsparse_matrix.h"


inline void computeStiffnessMatrix(
	const CMeMaterialProperty& mtl, const double3x3& XT, const int ii, const int jj, double3x3& stif)
{
	Vector3d *strain_deriv, *p;
	double3x3 s1[3], s2[3], stress[3];
	for (int i=0; i<3; i++){ 
		s1[i].setZeroMatrix(), s2[i].setZeroMatrix(); 
	}

	strain_deriv = (Vector3d *)(&XT.x[3*ii]);
	p=(Vector3d*)&s1[0].x[0]; *p = *strain_deriv; s1[0].AddTranspose(); 
	p=(Vector3d*)&s1[1].x[3]; *p = *strain_deriv; s1[1].AddTranspose(); 
	p=(Vector3d*)&s1[2].x[6]; *p = *strain_deriv; s1[2].AddTranspose(); 
	strain_deriv = (Vector3d *)(&XT.x[3*jj]);
	p=(Vector3d*)&s2[0].x[0]; *p = *strain_deriv; s2[0].AddTranspose(); 
	p=(Vector3d*)&s2[1].x[3]; *p = *strain_deriv; s2[1].AddTranspose(); 
	p=(Vector3d*)&s2[2].x[6]; *p = *strain_deriv; s2[2].AddTranspose(); 

	for (int i=0; i<3; i++)
		mtl.isotropicMaterialStress(s2[i], stress[i]);
	for (int i=0, index=0; i<3; ++i)
		for (int j=0; j<3; ++j, ++index)
			stif.x[index] = InnerProd(s1[i], stress[j]);
}

//shear element composed of two triangles
void CTetFemElement::_initElement(
	const CMeMaterialProperty& mtl,
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3)
{
	//precompute volume of the tet
	m_vol = computeTetrahedronVolume(p0, p1, p2, p3);

	//precompute the inverse of the initial config X
	CopyVectorsToColumns(p1-p0, p2-p0, p3-p0, m_matXT);
	m_matXT.Invert();

	//get stiffness matrix
	const double K = -m_vol*0.25;
	for (int i=0, index=0; i<3; i++){
		for (int j=0; j<3; j++, index++){
			computeStiffnessMatrix(mtl, m_matXT, i, j, m_matStiff[index]);
			m_matStiff[index]*=K;
		}
	}
}

//compute the elastic force using the deformation gradient, where 
//the rotation has been removed from it.
inline void CTetFemElement::_computeForceStatic(
	const double3x3& DG, const CMeMaterialProperty &mtl, Vector3d F[3])
{
	double3x3 strain, stress;	//strain and stress
	//infinitesimal strain
	ChauchyStrain(DG, strain);
	//SVK material stress
	double lambda, miu; 
	mtl.getLameCofficients(lambda, miu);
	const double vol = -m_vol;
	lambda*=vol, miu*=vol;
	CMeMaterialProperty::IsotropicMaterialStress(lambda, miu, strain, stress);
	//strain partial deriv
	Vector3d* deriv;
	deriv = (Vector3d*)(&m_matXT.x[0]); 
	ComputeElasticForce(*deriv, stress, F[0]);
	deriv = (Vector3d*)(&m_matXT.x[3]); 
	ComputeElasticForce(*deriv, stress, F[1]);
	deriv = (Vector3d*)(&m_matXT.x[6]); 
	ComputeElasticForce(*deriv, stress, F[2]);
}

//compute the rotation matrix for the element
//in the case of static deformation, the rotation is identity, otherwise, 
//the polar decomposition method is used.
//R: the rotation, S: the streching and shear component
inline void CTetFemElement::_computeRotation(
	const unsigned int timeid, 
	const int isstatic,
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	double3x3 &R,	//local to world rotation
	double3x3 &S)	//stretch matrix
{
	int rank, dim=3;
	double3x3 M;
	CopyVectorsToColumns(q1, q2, q3, M);
	const double3x3 DG = M*m_matXT;     //deformation gradient

	//for static deformation, no need to compute the rotation
	if (isstatic){
		R.setIdentityMatrix();
		S = DG;
		return;
	}
	
	//for large deformation, compute the roation and remove it for DG
	const double delta = DG.Det();
	if (delta<0){
		R = m_matRot;
	}
	else{
		const double ERRTOL = 1e-4;
		polar_decomp(dim, ERRTOL, DG.x, R.x, rank);
		m_matRot = R;
	}

	{ double3x3 RT = R; RT.Transpose(); S = RT*DG; }
}

void CTetFemElement::computeForce(
	const unsigned int timeid,	//time integration timestep ID
	const int isstatic,			//small or large deform?
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const CMeMaterialProperty &mtl, 
	Vector3d F[4],				//forces for the 4 nodes, all filled
	double3x3* jac[9])			//jacobian for p1, p2, p3 3X3
{
	//compute deformation gradient
	double3x3 RT;				//local to world transform
	double3x3 S;				//element stretching component
	const Vector3d q1 = p1 - p0;
	const Vector3d q2 = p2 - p0;
	const Vector3d q3 = p3 - p0;
	_computeRotation(timeid, isstatic, q1, q2, q3, RT, S);

	const double3x3 *M = m_matStiff;
	if (!isstatic){	//large deformation FEM
		//rotate world into local, then compute nodal forces
		double3x3 R = RT; R.Transpose();
		const Vector3d u1 = R*q1 - m_vert[1];
		const Vector3d u2 = R*q2 - m_vert[2];
		const Vector3d u3 = R*q3 - m_vert[3];
		F[1] = M[0]*u1 + M[1]*u2 + M[2]*u3;
		F[2] = M[3]*u1 + M[4]*u2 + M[5]*u3;
		F[3] = M[6]*u1 + M[7]*u2 + M[8]*u3;
		F[1] = RT*F[1], F[2] = RT*F[2], F[3] = RT*F[3];
		if (jac && jac[0]){				//rotate the jacobian		
			for (int i=0; i<9; i++)
				*jac[i] = RT*M[i]*R;	//rotated local stiffneM: K = R^T * k * R
		}
	}
	else{ //static FEM
		_computeForceStatic(S, mtl, &F[1]);
		if (jac && jac[0]){				//directly copy stiffneM
			for (int i=0; i<9; i++) 
				*jac[i] = M[i];
		}
	}

	F[0] = -(F[1]+F[2]+F[3]);
}


//Save the 12x12 matrix for the stiffness matrix;
//We only store the 9x9 matrix 
void CTetFemElement::saveStiffness(CSparseMatrix33 *p, const double3x3 jac[9])
{
	//store sub 3X3 block
	const int i0 = m_nodeID[0]; 
	const int i1 = m_nodeID[1];
	const int i2 = m_nodeID[2]; 
	const int i3 = m_nodeID[3];
	const double3x3& s11 = jac[0];
	const double3x3& s12 = jac[1];
	const double3x3& s13 = jac[2];
	const double3x3& s21 = jac[3];
	const double3x3& s22 = jac[4];
	const double3x3& s23 = jac[5];
	const double3x3& s31 = jac[6];
	const double3x3& s32 = jac[7];
	const double3x3& s33 = jac[8];
	p->addWithMatrixElement(i1, i1, s11);
	p->addWithMatrixElement(i1, i2, s12);
	p->addWithMatrixElement(i1, i3, s13);
	p->addWithMatrixElement(i2, i1, s21);
	p->addWithMatrixElement(i2, i2, s22);
	p->addWithMatrixElement(i2, i3, s23);
	p->addWithMatrixElement(i3, i1, s31);
	p->addWithMatrixElement(i3, i2, s32);
	p->addWithMatrixElement(i3, i3, s33);

	//store the first row/col
	double3x3 s01 = s11+s21+s31;
	double3x3 s02 = s12+s22+s32;
	double3x3 s03 = s13+s23+s33;
	double3x3 s00 = s01+s02+s03;
	s01=-s01, s02=-s02, s03=-s03;
	double3x3 s10 = s11+s12+s13;
	double3x3 s20 = s21+s22+s23;
	double3x3 s30 = s31+s32+s33;
	s10=-s10, s20=-s20, s30=-s30;
	p->addWithMatrixElement(i0, i0, s00);
	p->addWithMatrixElement(i0, i1, s01);
	p->addWithMatrixElement(i0, i2, s02);
	p->addWithMatrixElement(i0, i3, s03);
	p->addWithMatrixElement(i1, i0, s10);
	p->addWithMatrixElement(i2, i0, s20);
	p->addWithMatrixElement(i3, i0, s30);
}
