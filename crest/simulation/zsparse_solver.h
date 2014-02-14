//FILE: zsparse_matrix.h
//A sparse matrix definition, where each element is a 3x3 submatrix,
//It is used in the implicit ODE solver: backward Euler method.

#ifndef _ZSPARSE_SOLVER_ZN2009_H
#define _ZSPARSE_SOLVER_ZN2009_H

#include <stdio.h>
#include <memory.h>
#include <memmgr.h>
#include <matrix3x3.h>
#include "zsparse_matrix.h"
#include "zdiag_matrix.h"
#include "lsolver/cghs.h"
#include "lsolver/bicgsq.h"
#include "lsolver/gmres.h"


inline int SparseSolverCghs(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const CICPrecondMatrix& C,	//precondition matrix
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL,		//stopping criterion 
	const int ITRMAX=30)	//number of iterations
{
	const int n = A.size();	//dimension of the matrix 
	const int n3 = n*3; 
    return cghs(n3, ITRMAX, A, C, &b[0].x, &outvec[0].x, ETOL);
}

inline int SparseSolverCghs(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* C,		//precondition matrix
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL,		//stopping criterion 
	const int ITRMAX=30)	//number of iterations
{
	const int n = A.size();	//dimension of the matrix 
	const int n3 = n*3; 
	CDiagonalMatrix D(&C[0].x, n3, false);
    return cghs(n3, ITRMAX, A, D, &b[0].x, &outvec[0].x, ETOL);
}

inline int SparseSolverCghs(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL,		//stopping criterion 
	const int ITRMAX=60)	//number of iterations
{
	const int n =A.size()*3; //dimension of the matrix 
    return cghs(n, ITRMAX, A, &b[0].x, &outvec[0].x, ETOL);
}

extern int SparseSolverGmres(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL);	//stopping criterion 

extern int SparseSolverBicgstab(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL);	//stopping criterion 

extern int SparseSolverBicgsq(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL);	//stopping criterion 


class CSparseSolver
{
private:
	Vector3d *D1;
	Vector3d *b1;

private: 
	static double _computeError(const Vector3d *p, const Vector3d *q, const int n)
	{
		double e0=1e30;
		for (int i=0; i<n; i++){
			Vector3d dif = p[i] - q[i];
			double e = _MAX3_(fabs(dif.x), fabs(dif.y), fabs(dif.z));
			if (e<e0)
				e0=e;
		}
		return e0;
	}

	void _checkDiagonal(const Vector3d *D, const int n)
	{
		for (int i=0; i<n; i++){
			double x = fabs(D[i].x);
			double y = fabs(D[i].y);
			double z = fabs(D[i].z);
			double k = _MIN3_(x, y, z);
			if (k<1e-10){
				printf("Diagonal element is too small %lg\n", k);
			}
		}
	}

public:
	CSparseSolver(void)
	{
		D1 = NULL;
		b1 = NULL;
	}

	~CSparseSolver()
	{
		SafeDeleteArray(D1);
		SafeDeleteArray(b1);
	}

	//out vector should be initialized with the tiral values
	int solveGauss(CSparseMatrix33 &A, const Vector3d* b, Vector3d *out, const double& err, const int nitr)
	{
		int i;
		const int n = A.size();
		D1 = new Vector3d[n];
		b1 = new Vector3d[n];

		//compute A1 = I - D^-1 * A
		A.getDiagonalMatrix(D1);
		for (i=0; i<n; i++){
			D1[i].x = -1.0/D1[i].x; D1[i].y = -1.0/D1[i].y; D1[i].z = -1.0/D1[i].z;
		}
		A.multiplyDiagonalMatrixOnLeft(D1);
		A.addIdentityMatrix();

		//compute b1
		for (i=0; i<n; i++){
			b1[i].x = -D1[i].x*b[i].x; b1[i].y = -D1[i].y*b[i].y; b1[i].z = -D1[i].z*b[i].z;
		}
		
		Vector3d *Xk = out, *Xk1 = D1;
		for (int i=0; i<nitr; i++){
			A.multiplyVector(Xk, Xk1);
			for (int j=0; j<n; j++) Xk1[j]+=b1[j];
			Vector3d *t=Xk; Xk=Xk1; Xk1=t;
		}

		return i;
	}

	int solveJacobi(CSparseMatrix33 &A, const Vector3d* b, Vector3d *out, const double& ETOL, const int nitr)
	{
		int i;
		const int n = A.size();
		D1 = new Vector3d[n];
		b1 = new Vector3d[n];

		//compute A = D + A0
		A.extractDiagonalMatrix(D1);
		_checkDiagonal(D1, n);
		for (i=0; i<n; i++){
			D1[i].x = -1.0/D1[i].x; D1[i].y = -1.0/D1[i].y; D1[i].z = -1.0/D1[i].z;
		}
		A.multiplyDiagonalMatrixOnLeft(D1);

		//compute b1
		for (i=0; i<n; i++){
			b1[i].x = -D1[i].x*b[i].x; b1[i].y = -D1[i].y*b[i].y; b1[i].z = -D1[i].z*b[i].z;
		}

		double myerr;
		Vector3d *Xk = out, *Xk1 = D1;
		for (int i=0; i<nitr; i++){
			A.multiplyVector(Xk, Xk1);
			for (int j=0; j<n; j++) Xk1[j]+=b1[j];
			Vector3d *t=Xk; Xk=Xk1; Xk1=t;
			if (i>=17 && (i&0x1)==1){
				myerr= _computeError(Xk, Xk1, n);
				//printf("Iter %d error is %lg\n", i, e);
				if (myerr<ETOL) 
					return i;
			}
		}
		printf("The error remaining is %lg.\n", myerr);
		return i;
	}

	inline int solveGmres(CSparseMatrix33 &A, const Vector3d* b, Vector3d *outvec, const double& ETOL)
	{
		const int m = 20;
		const int n = A.size() * 3;
		int itr = gmres(m, n, A, (const double*)b, (double*)outvec, ETOL);
		return itr;
	}

};


#endif