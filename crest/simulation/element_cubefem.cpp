//FILE: element_cubefem.cpp
#include <geomath.h>
#include <decompose.h>			//Polar decomposition
#include "element_cubefem.h"
#include "vertexring_element.h"
#include "element_shear.h"
#include "zsparse_matrix.h"


inline void saveOneMat33(
	double3x3 K[8][8], 
	int i, int j, 
	double x00, double x01, double x02, 
	double x10, double x11, double x12, 
	double x20, double x21, double x22)
{
	double *x = K[i][j].x;
	x[0]=x00, x[1]=x01, x[2]=x02;
	x[3]=x10, x[4]=x11, x[5]=x12;
	x[6]=x20, x[7]=x21, x[8]=x22;
	if (i!=j){
		K[j][i]=K[i][j];	
		K[j][i].Transpose();
	}
}

//compute the stiffness matrix of a cube element
void buildCubeElementStiffness(
	const double &h, 		//cube edge length
	const double &E, 		//Young's modulus
	const double &nu, 		//poisson ratio \nu
	double3x3 K[8][8])		//stiffness matrix of the element
{
	const double a = h*E*(1-nu)/((1+nu)*(1-2*nu));
	const double b = h*E*nu/((1+nu)*(1-2*nu));
	const double c = h*E/(2*(1+nu));
	
	//build the diagonal elements
	{
		const double d = (a+2*c)/9.0;
		const double o = (b+c)/12.0;
		const double n = -o;
		saveOneMat33(K, 0, 0,  d, o, o,  o, d, o,  o, o, d);
		saveOneMat33(K, 1, 1,  d, n, n,  n, d, o,  n, o, d);
		saveOneMat33(K, 2, 2,  d, o, n,  o, d, n,  n, n, d);
		saveOneMat33(K, 3, 3,  d, n, o,  n, d, n,  o, n, d);
		saveOneMat33(K, 4, 4,  d, o, n,  o, d, n,  n, n, d);
		saveOneMat33(K, 5, 5,  d, n, o,  n, d, n,  o, n, d);
		saveOneMat33(K, 6, 6,  d, o, o,  o, d, o,  o, o, d);
		saveOneMat33(K, 7, 7,  d, n, n,  n, d, o,  n, o, d);	
	}
	{
		const double d1 = (-a+c)/9.0;
		const double d2 = (a-c)/18.0;
		const double o1 = (b-c)/12.0;
		const double o2 = (b+c)/24.0;
		const double n1 = -o1;
		const double n2 = -o2;
		saveOneMat33(K, 0, 1,  d1, o1, o1,  n1, d2, o2,  n1, o2, d2);
		saveOneMat33(K, 2, 3,  d1, o1, n1,  n1, d2, n2,  o1, n2, d2);
		saveOneMat33(K, 4, 5,  d1, o1, n1,  n1, d2, n2,  o1, n2, d2);
		saveOneMat33(K, 6, 7,  d1, o1, o1,  n1, d2, o2,  n1, o2, d2);
		
		saveOneMat33(K, 0, 3,  d2, n1, o2,  o1, d1, o1,  o2, n1, d2);
		saveOneMat33(K, 1, 2,  d2, o1, n2,  n1, d1, o1,  n2, n1, d2);
		saveOneMat33(K, 4, 7,  d2, n1, n2,  o1, d1, n1,  n2, o1, d2);
		saveOneMat33(K, 5, 6,  d2, o1, o2,  n1, d1, n1,  o2, o1, d2);
		
		saveOneMat33(K, 0, 4,  d2, o2, n1,  o2, d2, n1,  o1, o1, d1);
		saveOneMat33(K, 1, 5,  d2, n2, o1,  n2, d2, n1,  n1, o1, d1);
		saveOneMat33(K, 2, 6,  d2, o2, o1,  o2, d2, o1,  n1, n1, d1);
		saveOneMat33(K, 3, 7,  d2, n2, n1,  n2, d2, o1,  o1, n1, d1);
	}
	{
		const double d1 = (-2*a-c)/36.0;
		const double d2 = (a-4*c)/36.0;
		const double o1 = (b+c)/12.0;
		const double o2 = (b-c)/24.0;
		const double n1 = -o1;
		const double n2 = -o2;
		saveOneMat33(K, 0, 5,  d1, o2, n1,  n2, d2, n2,  n1, o2, d1);
		saveOneMat33(K, 1, 4,  d1, n2, o1,  o2, d2, n2,  o1, o2, d1);
		saveOneMat33(K, 1, 6,  d2, o2, o2,  n2, d1, n1,  n2, n1, d1);
		saveOneMat33(K, 2, 5,  d2, n2, o2,  o2, d1, o1,  n2, o1, d1);
		
		saveOneMat33(K, 2, 7,  d1, o2, o1,  n2, d2, o2,  o1, n2, d1);
		saveOneMat33(K, 3, 6,  d1, n2, n1,  o2, d2, o2,  n1, n2, d1);
		saveOneMat33(K, 0, 7,  d2, n2, n2,  o2, d1, n1,  o2, n1, d1);
		saveOneMat33(K, 3, 4,  d2, o2, n2,  n2, d1, o1,  o2, o1, d1);
		
		saveOneMat33(K, 0, 2,  d1, n1, o2,  n1, d1, o2,  n2, n2, d2);
		saveOneMat33(K, 1, 3,  d1, o1, n2,  o1, d1, o2,  o2, n2, d2);
		saveOneMat33(K, 4, 6,  d1, n1, n2,  n1, d1, n2,  o2, o2, d2);
		saveOneMat33(K, 5, 7,  d1, o1, o2,  o1, d1, n2,  n2, o2, d2);		
	}
	{
		const double d = (-a-2*c)/36.0;
		const double o = (b+c)/24.0;
		const double n = -o;
		saveOneMat33(K, 0, 6,  d, n, n,  n, d, n,  n, n, d);
		saveOneMat33(K, 1, 7,  d, o, o,  o, d, n,  o, n, d);
		saveOneMat33(K, 2, 4,  d, n, o,  n, d, o,  o, o, d);
		saveOneMat33(K, 3, 5,  d, o, n,  o, d, o,  n, o, d);
	}

	for (int i=0; i<8; ++i)
		for (int j=0; j<8; ++j) 
			K[i][j]=-K[i][j];
}

inline Vector3d GET_CUBE_CENTER(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7)
{
	//return p0;
	return (p0+p1+p2+p3+p4+p5+p6+p7)*0.1250;
}

void CCubeFemElement::init(const CMeMaterialProperty& mtl, const int nodeid[8], 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7, 
	void *usrdata)
{
	m_pUserData = reinterpret_cast<UserData*>(usrdata);

	//copy node IDs
	for (int i=0; i<8; i++) m_nodeID[i] = nodeid[i];

	//copy vertices by convert vertices in a local frame
	const Vector3d c=GET_CUBE_CENTER(p0, p1, p2, p3, p4, p5, p6, p7);
	m_vert[0] = p0 - c;  m_vert[1] = p1 - c;
	m_vert[2] = p2 - c;  m_vert[3] = p3 - c;
	m_vert[4] = p4 - c;  m_vert[5] = p5 - c;
	m_vert[6] = p6 - c;  m_vert[7] = p7 - c;

	m_matRot.setIdentityMatrix();
}

//compute the elastic force using the deformation gradient, where 
//the rotation has been removed from it.
inline void CCubeFemElement::_computeForceStatic(
	const Vector3d q[8], const CMeMaterialProperty &mtl, Vector3d F[8])
{
	Vector3d u[8];

	//compute forces for node 0--6
	for (int i=0; i<8; i++){
		F[i] = Vector3d(0);
		u[i] = q[i] - m_vert[i];
	}
	for (int i=0; i<7; i++){
		for (int j=0; j<8; j++){
			Vector3d v = u[j];
			//double t = v.y; v.y=v.z; v.z=t;
			F[i]+=m_pUserData->mStiffnessMatrix[i][j]*v;
		}
	}

	//compute force for node 7
	F[7]=F[0];
	for (int i=1; i<7; i++) F[7]+=F[i];
	F[7]=-F[7];
}

//compute the rotation matrix for the element
//in the case of static deformation, the rotation is identity, otherwise, 
//the polar decomposition method is used.
//R: the rotation, S: the streching and shear component
inline void CCubeFemElement::_computeRotation(
	const unsigned int timeid, 
	const int isstatic,
	const Vector3d q[8],
	double3x3 &R)		//local to world rotation
{
	//for static deformation, no need to compute the rotation
	if (isstatic){
		R.setIdentityMatrix();
		return;
	}

	//for large deformation, compute the roation and remove it for DG
	double3x3 A;
	const int nv = 8;
	const int stride = sizeof(Vector3d);
	double weights[8]={0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125, 0.125};
	const int strideW = sizeof(double);
	computeLeastSquareMatrix(nv, (const double*)q, stride, (const double*)m_vert, stride, weights, strideW, A);
	const double delta = A.Det();
	if (delta<0){
		R = m_matRot;
	}
	else{
		int rank;
		const double ERRTOL = 1e-4;
		polar_decomp(3, ERRTOL, A.x, R.x, rank);	
		m_matRot = R;
	}
}

void CCubeFemElement::computeForce(
	const unsigned int timeid,	//time integration timestep ID
	const int isstatic,			//small or large deform?
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const Vector3d &p4, const Vector3d &p5, const Vector3d &p6, const Vector3d &p7, 
	const CMeMaterialProperty &mtl, 
	Vector3d F[8],				//forces for the 8 nodes, all filled
	double3x3* jac[64])			//jacobian for all 8x8 block matrix
{
	const Vector3d c=GET_CUBE_CENTER(p0, p1, p2, p3, p4, p5, p6, p7);
	const Vector3d q[8]={p0-c, p1-c, p2-c, p3-c, p4-c, p5-c, p6-c, p7-c};

	//if (1){						//small deformation FEM
	if (isstatic){			//small deformation FEM
		_computeForceStatic(q, mtl, F);
		if (jac && jac[0]){		//directly copy stiffneM
			for (int i=0, ind=0; i<8; ++i){
				for (int j=0; j<8; ++j, ++ind)
					*jac[ind] = m_pUserData->mStiffnessMatrix[i][j];
			}
		}
		return;
	}

	//compute deformation gradient
	double3x3 RT;				//local to world transform
	_computeRotation(timeid, isstatic, q, RT);

	const double3x3 *M = 0;		//m_matStiff;
	//rotate world into local, then compute nodal forces
	double3x3 R = RT; R.Transpose();
	Vector3d qq[8];
	for (int i=0; i<8; i++) qq[i]=R*q[i];
	_computeForceStatic(qq, mtl, F);
	for (int  i=0; i<8; i++) F[i]=RT*F[i];
	if (jac && jac[0]){			//rotate the jacobian	
		for (int i=0, ind=0; i<8; ++i){
			for (int j=0; j<8; ++j, ++ind)
				*jac[ind] = RT*m_pUserData->mStiffnessMatrix[i][j]*R;
		}
	}
}


//Save the 12x12 matrix for the stiffness matrix;
//We only store the 9x9 matrix 
void CCubeFemElement::saveStiffness(CSparseMatrix33 *p, const double3x3 jac[64])
{
	for (int i=0, k=0; i<8; ++i){
		const int ii = m_nodeID[i];
		for (int j=0; j<8; ++j, ++k){
			const int jj = m_nodeID[j];
			p->addWithMatrixElement(ii, jj, jac[k]);
		}
	}
}
