//FILE: zdiag_matrix.h
//A sparse matrix definition, where each element is a 3x3 submatrix,
//It is used in the implicit ODE solver: backward Euler method.

#ifndef _INC_ZDIAGONAL_MATRIX_ZN2009_H
#define _INC_ZDIAGONAL_MATRIX_ZN2009_H

#include <assert.h>
#include <stdio.h>
#include <vector3d.h>


class CDiagonalMatrix
{
protected:
	int m_nv;
	double *m_pBuffer;
	bool m_bOwnMemory;

public:
	CDiagonalMatrix(void)
	{
		m_nv = 0;
		m_bOwnMemory = false;
		m_pBuffer = NULL;
	}

	void init(const double *p, const int nv, const bool allocBuffer)
	{
		m_nv = nv;
		if (allocBuffer){
			m_bOwnMemory = true;
			m_pBuffer = new double [nv];
			assert(m_pBuffer!=NULL);
			for (int i=0; i<nv; i++) m_pBuffer[i]=p[i];
		}
		else{
			m_bOwnMemory = false;
			m_pBuffer = (double*) p;
		}
	}

	CDiagonalMatrix(const double *p, const int nv, const bool allocBuffer=false)
	{
		init(p, nv, allocBuffer);
	}

	~CDiagonalMatrix(void)
	{
		if (m_bOwnMemory && m_pBuffer!=NULL) 
			delete [] m_pBuffer;
	}

	friend inline void precond_mult(const CDiagonalMatrix& A, const double *x, const int N, double *y)
	{
		const double *s = A.m_pBuffer;
		if (A.m_nv==N){
			for (int i=0; i<N; i++) y[i]=x[i]*s[i];
			return;
		}
		if ((A.m_nv*3)==N){
			const Vector3d *xx = (const Vector3d *)x;
			Vector3d *yy = (Vector3d *)y;
			for (int i=0; i<A.m_nv; i++) yy[i]=xx[i]*s[i];
			return;
		}
		assert(0);
	}
};


#endif