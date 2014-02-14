//FILE: zicprecond_matrix.cpp
#include "zicprecond_matrix.h"
#include <ztime.h>

inline void _CompRowLowerDiagSolve(const int m, 
	const double *val, const int *indx, const int *pntr,
    const double *b, double *c)
{
	ASSERT0(b==c);
	for (int i=0; i<m; i++) {
		double z = 0;
		for (int j = pntr[i]; j < pntr[i+1] - 1; j++)
			z += c[indx[j]] * val[j];
		c[i+pntr[0]] = (b[i] - z) / val[pntr[i+1]-1];
    }
}

inline void _CompRowUpperDiagSolve(int m, 
	const double *val, const int *indx, const int *pntr, const Vector3d *b, Vector3d *c)
{
	for (int i=m-1; i>=0; i--) {    
		Vector3d z(0);
		for (int j=pntr[i]+1; j<pntr[i+1]; j++)
			z += c[indx[j]] * val[j];
		c[i+pntr[0]] = (b[i] - z) / val[pntr[i]];
	}
}

inline void _CompColLowerDiagSolve(int m, 
	const double *val, const int *indx, const int *pntr, const Vector3d *b, Vector3d *c)
{
	memcpy(c, b, sizeof(Vector3d)*m);
	for (int i = 0; i < m; i++) {
		Vector3d z = c[i+pntr[0]] / val[pntr[i]];
		c[i+pntr[0]] = z;
		for (int j = pntr[i] + 1; j < pntr[i+1]; j++)
			c[indx[j]] -= z * val[j];
	}
}

/*
void mult(const CICPrecondMatrix& A, const double *x, const int N, double *y)
{
	int i, j;
	const int n = A.m_nv;
	const int n3 = n*3;
	if (n3 == N){
		ASSERT0(A.m_pBuffer!=NULL);
		double *pSrc=A.m_pBuffer;
		MV_Vector_double X(pSrc, n, (MV_Vector_::ref_type)1);

		double *val = &A.m_icp->val_(0);
		int *pntr = &A.m_icp->pntr_(0);
		int *indx = &A.m_icp->indx_(0);
	//=================================================================
		#define IC_SOLVE_VECTOR_COMPONENT(K)			 \
		for (i=0,j=K; i<n; i++,j+=3) pSrc[i]=x[j];		 \
		_CompColLowerDiagSolve(n, val, indx, pntr, pSrc, pSrc);\
		_CompRowUpperDiagSolve(n, val, indx, pntr, pSrc, pSrc);\
		for (i=0,j=K; i<n; i++,j+=3) y[j]=pSrc[i];	
	//==================================================================
		IC_SOLVE_VECTOR_COMPONENT(0);
		IC_SOLVE_VECTOR_COMPONENT(1);
		IC_SOLVE_VECTOR_COMPONENT(2);
		#undef IC_SOLVE_VECTOR_COMPONENT
	}
	else{
		assert(0);
	}
}

*/

void precond_mult(const CICPrecondMatrix& A, const double *b, const int N, double *x)
{
	const int n = A.m_nv;
	const int n3 = n*3;
	if (n3 == N){
		double *val = &A.m_icp->val_(0);
		int *pntr = &A.m_icp->pntr_(0);
		int *indx = &A.m_icp->indx_(0);
		Vector3d *vb = (Vector3d*)b;
		Vector3d *vx = (Vector3d*)x;
		_CompColLowerDiagSolve(n, val, indx, pntr, vb, vx);
		_CompRowUpperDiagSolve(n, val, indx, pntr, vb, vx);
		return;
	}
	assert(0);
}

void CICPrecondMatrix::init(CSparseMatrix33 &m)
{
	const int M = m.size();
	const int N = M;
	const int nz = m.getElementCount();

	//In 1st first step, convert the SparseMatrix33 into row majored rep.
	m_nv = M;
	double *val = new double[nz];
	int *row = new int [M+2];			//real size should be M+1
	int *col = new int [nz];
	assert((val!=NULL) && (row!=NULL) && (col!=NULL));
	double3x3* e0 = m.getMatrixElement(0,0);
	double sign = 1;
	if (e0->x[0]<0) sign = -1;

	int i, c;
	for (i=c=row[0]=0; i<M; i++){
		double *dst = val+c;
		const int n = m.getAllElementsInOneRow(i, col+c, dst);
		for (int j=0; j<n; j++) dst[j]=dst[j] * sign;
		c+=n, row[i+1]=c;
	}
	assert(c==nz);
	CompRow_Mat_double A(M, N, nz, val, row, col);
	delete [] val;
	delete [] row;
	delete [] col;

	//then, we construct the IC preconditioner
	const int timerid = 3;
	startFastTimer(timerid);
	m_icp = new ICPreconditioner_double(A);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Incomplete Cholesky run time:");
}
