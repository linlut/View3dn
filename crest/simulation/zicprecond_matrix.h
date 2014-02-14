//FILE: zImcompleteCholeskyPreconditioner_matrix.h
//A sparse matrix definition, where each element is a 3x3 submatrix,
//It is used in the implicit ODE solver: backward Euler method.

#ifndef _ZICPRECOND_MATRIX_ZN2009_H
#define _ZICPRECOND_MATRIX_ZN2009_H

#include <assert.h>
#include <stdio.h>
#include <vector3d.h>
#include "zsparse_matrix.h"
#include "SparseLib++/include/icpre_double.h"


class CSparseMatrix33;


class CICPrecondMatrix
{
protected:
	int m_nv;
	ICPreconditioner_double *m_icp;

private:
	void _setZero(void)
	{
		m_nv = 0;
		m_icp = NULL;
	}

public:
	CICPrecondMatrix(void)
	{
		_setZero();	
	}

	~CICPrecondMatrix(void)
	{
		SafeDelete(m_icp);
	}

	void init(CSparseMatrix33 &m);

	CICPrecondMatrix(CSparseMatrix33 &m)
	{
		_setZero();	
		init(m);
	}

	friend void precond_mult(const CICPrecondMatrix& A, const double *x, const int N, double *y);

};


#endif