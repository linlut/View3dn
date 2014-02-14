//FILE: vertexring_element.cpp
#include <decompose.h>				//Polar decomposition
#include <eigenvalue.h>
#include <quaternion.h>
#include "vertexring_element.h"

const int VERTEX_BUFFER_LENGTH = 60;

static inline double l2_norm(const double3x3 &a, const double3x3 &b)
{
	int i;
	const double *x = a.x;
	const double *y = b.x;
	double sum = 0;
	for (i=0; i<9; i++){
		const double s = x[i]-y[i];
		sum+= s*s;
	}
	return sum;
}

static inline Vector3d 
_getVertex3dUsingStride(const void *parray, const int n, const int stride)
{
	const char *p = (const char *)parray;
	p += stride*n;
	return *((const Vector3d *)p);
}

static inline double 
_getDoubleUsingStride(const void *parray, const int n, const int stride)
{
	const char *p = (const char *)parray;
	p += stride*n;
	return *((const double *)p);
}

inline void CVertexRingElement::_getLoacalVertexPositions(
	const int stride, const Vector3d *parray, Vector3d *pOneRingNodePos) const
{
	const Vector3d p0 = _getVertex3dUsingStride(parray, m_nVertexID, stride);
	for (int i=0; i<m_nv; i++){
		const int j = m_pVertexRingNode[i].m_nVertexID;
		pOneRingNodePos[i] = _getVertex3dUsingStride(parray, j, stride) - p0;
	}
}

//Directly using Gyrod elements for the computation
inline void CVertexRingElement::_compute1RingForceStatic(
	const Vector3d localpos[],			//1ring nodes' current local positions
	Vector3d force[],					//force array, buffer is for one-ring nodes
	double3x3 *stiffi[])				//jacobian array, buffer is for one-ring nodes
{
	for (int i=0; i<m_nv; i++){
		CVertexRingNode& node = m_pVertexRingNode[i];
		CMeMaterialProperty &m = *(node.m_pMaterial);
		node.computeNodalForce(localpos[i], m, force[i], stiffi[i]);
	}
}

inline void CVertexRingElement::_compute1RingForceDynamic(
	const int needCompRotation, 
	const Vector3d ringnodepos[],		//1ring nodes' current positions in local coordiante
	Vector3d F[],						//force array, in world coordinate
	double3x3 *stiffi[])				//jacobian array
{
	//need to copmute the rotation first, there are several methods
	if (needCompRotation){
		m_R0 = m_R;
		const bool r = computeRotationMatrix(ringnodepos, m_R);
		if (!r) m_R = m_R0;
	}
	for (int i=0; i<m_nv; i++){
		CVertexRingNode& node = m_pVertexRingNode[i];
		CMeMaterialProperty &mtl = *(node.m_pMaterial);
		node.computeNodalForce(ringnodepos[i], m_R, mtl, F[i], stiffi[i]);
	}
}

void CVertexRingElement::computeForce(
	const int needCompRotation, 
	const int isStatic,					//static simulation or dynamic simu	
	const Vector3d p[],					//nodes' current positions
	const int stride,					//consider nodes are packed in an array which are not continious
	const CMeMaterialProperty* pmtl,	//material
	Vector3d F[],						//force array
	double3x3 *ppJacobian[])			//jacobian array
{
	Vector3d *F1 = &F[1];
	double3x3 **ppJacobian1 = ppJacobian+1;	
	Vector3d nodepos[VERTEX_BUFFER_LENGTH];

	//copy the vertex positions
	_getLoacalVertexPositions(stride, p, nodepos);

	if (isStatic){
		_compute1RingForceStatic(nodepos, F1, ppJacobian1);
	}
	else{
		_compute1RingForceDynamic(needCompRotation, nodepos, F1, ppJacobian1);
	}

	//the summed force for the center node
	F[0] = -F1[0];
	for (int i=1; i<m_nv; i++) F[0] -= F1[i];
}


void CVertexRingElement::computeJacobianSymbolic(
	const CMeMaterialProperty* pmtl,
	double3x3 pJacobianLambda[], double3x3 pJacobianMu[])
{
	for (int i=0; i<m_nv; ++i){
		CVertexRingNode& node = m_pVertexRingNode[i];
		node.computeJacobianSymbolic(pmtl, pJacobianLambda[i], pJacobianMu[i]);
	}
}


//================Gen the rotation matrix=========================
//compute optimal rotation using iterative Polar Decomposition
//where the rotation is assuemd to be small, A is close to I
inline void _polarDecomposeItrSmall(const double3x3 &J, double3x3 &rot)
{
	double3x3 Rt, A;
	//in file matrix3x3dORGE.cpp
	//extern void QDUDecomposition(const double inmat[9], double outmat[9]);
	//QDUDecomposition(J.x, A.x);
	//A.Transpose();
	Rt = A;   Rt.Invert(); Rt.Transpose(); rot=0.5*(A+Rt);		//1
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//2
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//3
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//4
}

inline void _polarDecomposeItrLarge(const double3x3 &A, double3x3 &rot)
{
	double3x3 Rt;
	Rt = A;   Rt.Invert(); Rt.Transpose(); rot=0.5*(A+Rt);		//1
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//2
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//3
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//4
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//5
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//6
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//7
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//8
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//9
	Rt = rot; Rt.Invert(); Rt.Transpose(); rot=0.5*(rot+Rt);	//10
}

//B = A * S, where S is a 3x3 scaling matrix
inline void multMatrixScaleMat(const double3x3 &A, const Vector3d& S, double3x3& B)
{
	const double *a = A.x;
	double *b = B.x;
	b[0] = a[0]*S.x; b[1] = a[1]*S.y; b[2] = a[2]*S.z;
	b[3] = a[3]*S.x; b[4] = a[4]*S.y; b[5] = a[5]*S.z;
	b[6] = a[6]*S.x; b[7] = a[7]*S.y; b[8] = a[8]*S.z;
}

inline void _computeLeastSquareMatrix(
	const int nv, 
	const double *_P, const int strideP, 
	const double *_Q, const int strideQ, 
	const double *_Weight, const int strideW, 
	double3x3 &A)
{
	double3x3 r;
	Vector3d p = _getVertex3dUsingStride(_P, 0, strideP);
	Vector3d q = _getVertex3dUsingStride(_Q, 0, strideQ);
	const double w = _getDoubleUsingStride(_Weight, 0, strideW);
	VectorTensorProduct(p*w, q, A);

	for (int i=1; i<nv; i++){
		p = _getVertex3dUsingStride(_P, i, strideP);
		q = _getVertex3dUsingStride(_Q, i, strideQ);
		const double w = _getDoubleUsingStride(_Weight, i, strideW);
		VectorTensorProduct(p*w, q, r);
		A += r;
	}
}

void computeLeastSquareMatrix(
	const int nv, 
	const double *_P, const int strideP, 
	const double *_Q, const int strideQ, 
	const double *_Weight, const int strideW, 
	double3x3 &A)
{
	_computeLeastSquareMatrix(nv, _P, strideP, _Q, strideQ, _Weight, strideW, A);
}

inline void _computeReflectedLeastSquareMatrix(
	const int nv, 
	const double *_P, const int strideP, 
	const double *_Q, const int strideQ, 
	const double *_Weight, const int strideW, 
	const Vector3d &n,
	double3x3 &A)
{
	A.setZeroMatrix();
	for (int i=0; i<nv; i++){
		Vector3d p = _getVertex3dUsingStride(_P, i, strideP);
		Vector3d q = _getVertex3dUsingStride(_Q, i, strideQ);
		const double w = _getDoubleUsingStride(_Weight, i, strideW);
		Vector3d &s = q;
		const double dist = DotProd(s, n)*2.0;
		s -= dist*n;
		double3x3 r; 
		VectorTensorProduct(p*w, q, r);
		A += r;
	}
}

//input vertex number is 2, constant
inline void computePlanarRotationMatrix(
	const double *_Q, const int strideQ, const double *_Weight, const int strideW, 
	double3x3 &rot)
{
	Vector3d& X = *((Vector3d *)&rot.x[0]);
	Vector3d& Y = *((Vector3d *)&rot.x[3]);
	Vector3d& Z = *((Vector3d *)&rot.x[6]);
	Vector3d q1 = _getVertex3dUsingStride(_Q, 0, strideQ);
	Vector3d q2 = _getVertex3dUsingStride(_Q, 1, strideQ);
	const double w1 = _getDoubleUsingStride(_Weight, 0, strideW);
	const double w2 = _getDoubleUsingStride(_Weight, 1, strideW);
	Z = CrossProd(q1, q2);
	const double l2 = Magnitude2(Z);
	if (l2<1e-32){
		X = q1; X.normalize();
		const double nx = fabs(X.x);
		const double ny = fabs(X.y);
		const double nz = fabs(X.z);
		const double maxnxyz = _MAX3_(nx, ny, nz);
		if (nx==maxnxyz) 
			Z = Vector3d(0,0,1);
		else if (ny==maxnxyz) 
			Z = Vector3d(0,0,1);
		else 
			Z = Vector3d(-1,0,0);
		Y = CrossProd(Z, X); Y.normalize();
		Z = CrossProd(X, Y);
	}
	else{
		Z *= 1.0/sqrt(l2); //normalize Z
		q1.normalize(); q2.normalize();
		Y = w1*q1+w2*q2; Y.normalize();
		X = CrossProd(Y, Z);
	}
}

inline void CVertexRingElement::_computeRotationMatrix(
	const Vector3d *vertexpos, double3x3& A, double3x3& rot, int& rank) const
{
	const int strideP = sizeof(CVertexRingNode);
	const int strideQ = sizeof(Vector3d);
	const int strideW = sizeof(CVertexRingNode);
	const double *P = &m_pVertexRingNode[0].m_lpos0.x;
	const double *Q = &vertexpos[0].x;
	const double *Weight = &m_pVertexRingNode[0].m_weight;

	ASSERT0(m_nv>=2);
	if (m_nv==2){
		//BUG here, need to modify
		computePlanarRotationMatrix(Q, strideQ, Weight, strideW, rot);
		rank = 2;
	}
	else{
		_computeLeastSquareMatrix(m_nv, P, strideP, Q, strideQ, Weight, strideW, A);
		const double ERRTOL = 1e-4;
		polar_decomp(3, ERRTOL, A.x, rot.x, rank);
	}
}

//================================================================
void CVertexRingElement::init(
	const int vid, const Vector3d& pos, 
	const int nRod, const int pRingNodeID[], 
	const CMeMaterialPropertyPtr pMaterial[], 
	const Vector3d pRingNodePos[], const double edgevol[],
	CMemoryMgr &mgr)
{
	m_nVertexID = vid;
	m_nv = nRod;
	//allocate ring buffer
	m_pVertexRingNode = (CVertexRingNode*)mgr.Malloc(sizeof(CVertexRingNode)*nRod);
	//compute weight
	double weight[VERTEX_BUFFER_LENGTH];
	double totalE = 0;
	for (int i=0; i<nRod; i++) totalE += pMaterial[i]->getYoung();
	for (int i=0; i<nRod; i++) weight[i] = pMaterial[i]->getYoung()/totalE;
	
	if (m_nv >= 3){
		for (int i=0; i<nRod; i++){
			CVertexRingNode& e = m_pVertexRingNode[i];
			const Vector3d lpos = pRingNodePos[i] - pos;
			e = CVertexRingNode(pRingNodeID[i], pMaterial[i], lpos, edgevol[i], weight[i]);
		}
	}
	else{
		//BUG: here it is incompatible with the new method which computes edge forces only once 
		//need to modify this 
		assert(nRod == 2);
		Vector3d v[2];
		double3x3 rot;
		v[0]=pRingNodePos[0]-pos; v[1]=pRingNodePos[1]-pos;
		computePlanarRotationMatrix((const double*)v, sizeof(Vector3d), weight, sizeof(double), rot);
		v[0]=rot*v[0]; v[1]=rot*v[1];
		for (int i=0; i<nRod; i++){
			CVertexRingNode& e = m_pVertexRingNode[i];
			const Vector3d lpos = pRingNodePos[i] - pos;
			e = CVertexRingNode(pRingNodeID[i], pMaterial[i], v[i], edgevol[i], weight[i]);
		}
	}
	m_R.setIdentityMatrix();
	m_R0.setIdentityMatrix();
	m_quat = IdentityQuat;
	m_quat0 = IdentityQuat;
}

//================================================================

bool CVertexRingElement::computeRotationMatrix(const Vector3d* vertexpos, double3x3& rotation)
{
	bool r = true;
	int rank;
	double3x3* rot = &rotation;
	double3x3 A;

	//the rotation comp. is successful only when delta(rot)>0
	_computeRotationMatrix(vertexpos, A, *rot, rank);
	//assume small rotation btw consecutive steps, make corrections
	if (rank==3){
		const double delta = rot->Det();
		if (delta<0){
			r = false;
			//printf("Nagative matrix found in Polar Decomp! VID: %d\n", this->m_nVertexID);
		}
	}
	else{		
		ASSERT0(rank==2);
		Vector3d *rowa, *rowb;
		rowa = (Vector3d *)(&rot->x[0]), rowb=(Vector3d *)(&m_R0.x[0]);
		if (DotProd(*rowa, *rowb)<=0) 
			(*rowa) = -(*rowa);
		rowa = (Vector3d *)(&rot->x[3]), rowb=(Vector3d *)(&m_R0.x[3]);
		if (DotProd(*rowa, *rowb)<=0) 
			(*rowa) = -(*rowa);
		rowa = (Vector3d *)(&rot->x[6]), rowb=(Vector3d *)(&m_R0.x[6]);
		if (DotProd(*rowa, *rowb)<=0) 
			(*rowa) = -(*rowa);
	}
	return r;
}

bool CVertexRingElement::_updateRotationMatrixSolidOrShell(const int isshell, const Vector3d* vertexpos)
{
	const double ERRTOL = 1e-4;
	double3x3 A;
	int rank, dim=3;
	m_quat0 = m_quat;	//save last quat, also use it as the initial guess
	m_R0 = m_R;

	//asemble the matrix
	const int strideP = sizeof(CVertexRingNode);
	const int strideQ = sizeof(Vector3d);
	const int strideW = sizeof(CVertexRingNode);
	const double *P = &m_pVertexRingNode[0].m_lpos0.x;
	const double *Q = &vertexpos[0].x;
	const double *Weight = &m_pVertexRingNode[0].m_weight;
	_computeLeastSquareMatrix(m_nv, P, strideP, Q, strideQ, Weight, strideW, A);
	//test the sign of delta
	const double delta = A.Det();
	if (delta>=0){
		//polar_decomp(dim, ERRTOL, A.x, m_R.x, rank);
		//return true;

		double Sxx=A.x[0], Sxy=A.x[1], Sxz=A.x[2];
		double Syx=A.x[3], Syy=A.x[4], Syz=A.x[5];
		double Szx=A.x[6], Szy=A.x[7], Szz=A.x[8];
		Matrix2d m;
		m.x[0][0]=Sxx + Syy + Szz, m.x[0][1]=Syz - Szy, m.x[0][2]=Szx - Sxz, m.x[0][3]=Sxy - Syx;
		m.x[1][0]=m.x[0][1], m.x[1][1]=Sxx - Syy - Szz, m.x[1][2]=Sxy + Syx, m.x[1][3]=Szx + Sxz;
		m.x[2][0]=m.x[0][2], m.x[2][1]=m.x[1][2]; m.x[2][2]= - Sxx + Syy - Szz, m.x[2][3]= Syz + Szy;
		m.x[3][0]=m.x[0][3], m.x[3][1]=m.x[1][3], m.x[3][2]=m.x[2][3], m.x[3][3]= - Sxx - Syy + Szz;
		const double ERRTOL = 1e-8;
		double eigenvalue;
		Vector4d quat, guess(m_quat.w, m_quat.x, m_quat.y, m_quat.z);		
		const bool r = RayleighQuotientIteration(m, guess, ERRTOL, quat, eigenvalue);
		m_quat.x=quat[1], m_quat.y=quat[2], m_quat.z=quat[3], m_quat.w=quat[0]; 
		Quaternion *pquat = (Quaternion*)&m_quat.x;
		typedef double M33[3][3];
		pquat->getRotationMatrix(*((M33*)&m_R.x));
		return r;
	}
	//if solid, use lazzy approach to do nothing, since we don't know the axis to flip
	if (!isshell){
		return false;
	}

	//compute relfection of the shell vertices using vertex normal
	bool r = true;
	const double MATRIXDIST = 1.0;
	Vector3d norm = m_normal;
	_computeReflectedLeastSquareMatrix(m_nv, P, strideP, Q, strideQ, Weight, strideW, norm, A);
	polar_decomp(dim, ERRTOL, A.x, m_R.x, rank);
	const double l2dist = l2_norm(m_R, m_R0);
	if (l2dist>=MATRIXDIST) r=false;
	return r;
}

int CVertexRingElement::findCommonNeighbors(
	const CVertexRingElement& e0, const int noinclude_id, int buffer[])
{
	int nv = 0;
	for (int i=0; i<m_nv; i++){
		int vid = m_pVertexRingNode[i].m_nVertexID;
		if (vid==noinclude_id) continue;
		for (int j=0; j<e0.m_nv; j++){
			if (vid==e0.m_pVertexRingNode[j].m_nVertexID)
				buffer[nv++] = vid;
		}
	}
	return nv;
}

void CVertexRingElement::addOneNeighorNodeInCaseOnlyTwoNeighbors(
	const Vector3d *pVertex, const int vid2, CMemoryMgr &mgr)
{
	assert(m_nv == 2);
	m_nv = 3;
	CVertexRingNode &node0 = m_pVertexRingNode[0];
	CVertexRingNode &node1 = m_pVertexRingNode[1];
	const int vid0 = node0.m_nVertexID;
	const int vid1 = node1.m_nVertexID;
	int pRingNodeID[3] = {vid0, vid1, vid2};
	CMeMaterialPropertyPtr pMaterial[3] = {node0.m_pMaterial, node1.m_pMaterial, node1.m_pMaterial}; 
	Vector3d pRingNodePos[3] = {pVertex[vid0], pVertex[vid1], pVertex[vid2]};
	double edgevol[3] = {node0.getVolume(), node1.getVolume(), (node0.getVolume()+node1.getVolume())*0.5};
	this->init(m_nVertexID, pVertex[m_nVertexID], 3, pRingNodeID, pMaterial, pRingNodePos, edgevol, mgr);
}


bool CVertexRingElement::computeRotationQuaternionSolidOrShell(
		const int isshell, const Vector3d* vertexpos, const int stride)
{
	bool r = true;
	ASSERT0(m_nv>=3);
	//compute current rotation
	Vector3d nodepos[100];
	_getLoacalVertexPositions(stride, vertexpos, nodepos);
	//the rotation comp. is successful only when delta(rot)>0
	if (!_updateRotationMatrixSolidOrShell(isshell, nodepos)){		
		r = false;
	}
	else{//transform into quat.
		//Quaternion *pquat = (Quaternion*)&m_quat.x;
		//pquat->setFromRotationMatrix(*((double(*)[3][3])m_R.x));
		//pquat->invert();
	}
	return r;
}
