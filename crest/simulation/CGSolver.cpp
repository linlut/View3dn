#include "CGSolver.h"

CGSolver::CGSolver(SparseMatrix * matrix_): matrix(matrix_) 
{
  n = matrix->Getn();
  // create buffers
  r = (double*) malloc (sizeof(double) * n);
  d = (double*) malloc (sizeof(double) * n);
  q = (double*) malloc (sizeof(double) * n);
}

CGSolver::~CGSolver()
{
  free(r);
  free(d);
  free(q);
}

double CGSolver::DotProduct(double * x, double * y)
{
  double result = 0;
  for(int i=0; i<n; i++)
    result += x[i] * y[i];

  return result;
}

int CGSolver::CGSolve(double * b, double * x, double eps, int maxIter)
{
  int iter=1;
  matrix->MultiplyVector(x,r);
  int i;
  for (i=0; i<n; i++)
  {
    r[i] = b[i] - r[i];
    d[i] = r[i];
  }

  double deltaNew = DotProduct(r,r);
  double delta0 = deltaNew;

  while ((iter <= maxIter) && (deltaNew > eps * eps * delta0))
  {
    matrix->MultiplyVector(d,q);
    double dtq = DotProduct(d,q);
    double alpha = deltaNew / dtq;

    for(i=0; i<n; i++)
      x[i] += alpha * d[i];

    if (iter % 50 == 0)
    {
      matrix->MultiplyVector(x,r);
      for (i=0; i<n; i++)
        r[i] = b[i] - r[i];
    }
    else
    {
      for (i=0; i<n; i++)
        r[i] = r[i] - alpha * q[i];
    }

    double deltaOld = deltaNew;
    deltaNew = DotProduct(r,r);
    double beta = deltaNew / deltaOld;

    for (i=0; i<n; i++)
      d[i] = r[i] + beta * d[i];

    iter++;
  }

  return iter;
}

