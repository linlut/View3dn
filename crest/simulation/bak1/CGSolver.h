/*
  A rudimentary conjugate gradient solver (without preconditioning), 
  build on top of the sparse matrix class.

  The sparse matrix must be symmetric and positive-definite.

  The CG Solver was implemented by following Jonathan Shewchuk's 
  An Introduction to the Conjugate Gradient Method Without the Agonizing Pain
  http://www.cs.cmu.edu/~jrs/jrspapers.html#cg

  Code author: Jernej Barbic

  See sparseMatrix.h for licensing information.
*/

#ifndef _CGSOLVER_H_
#define _CGSOLVER_H_

#include "sparseMatrix.h"

class CGSolver
{
public:
  CGSolver(SparseMatrix * matrix);
  ~CGSolver();

  // solves M * x = b
  // x should (on input) contain the initial guess
  // on output, x will be modified to the new value (the solution)
  // 0 < eps < 1 is the error tolerance
  // maxIter is the max number of CG iterations
  // return value is the number of iterations performed until convergence
  // (if return value is maxIter, the solver did not converge)
  int CGSolve(double * b, double * x, double eps=1e-6, int maxIter=100);

  double DotProduct(double * x, double * y);

protected:
  int n;
  SparseMatrix * matrix; 
  double * r, * d, * q;
};

#endif

