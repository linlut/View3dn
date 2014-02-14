
/*! \file etkMatrix.cpp
    \brief Matrix structure and operations.
 */

#include <stdio.h>
#include <memory.h>
#include "etkErrorAPI.hpp"
#include "etkMatrix.hpp"

/// ---------------------------------------------------------------------------
/// Print out a matrix

void mprint (matrixptr m)
{
  char buffer [1000];

  for (int i = 0; i < m->rows; i++)
    {
      sprintf (buffer, "\n");
      for (int j = 0; j < m->cols; j++)
      {
      sprintf (buffer, "%s %8.2lf", buffer, *(m->block + i * m->cols + j));
      }
    }
  etkMessage (buffer);
}

/// ---------------------------------------------------------------------------
/// Add matrix m1 to m2 giving dm

inline int madd(matrixptr m1, matrixptr m2, matrixptr dm)
{
  if (m1->rows!=m2->rows||m1->cols!=m2->cols)
    etkError ("madd error - matrices different sizes");

  if (m1->rows!=dm->rows||m1->cols!=dm->cols)
    etkError ("nmadd error - matrices different sizes");

  for (int i=0;i<m1->rows;i++)
    for (int j=0;j<m1->cols;j++)
      *(dm->block+i*m1->cols+j)=*(m1->block+i*m1->cols+j)+\
      *(m2->block+i*m1->cols+j);

  return(0);
}

/// ---------------------------------------------------------------------------
/// Copy matrix sm to dm

inline int mcopy (matrixptr sm,matrixptr dm)
{
  if (dm->rows!=sm->rows||dm->cols!=sm->cols)
    etkError ("mcopy error - matrices different sizes");

  for (int i=0;i<dm->rows;i++)
    for (int j=0;j<dm->cols;j++)
      *(dm->block+i*dm->cols+j)=*(sm->block+i*dm->cols+j);

  return(0);
}

/// ---------------------------------------------------------------------------
/// Scalar multiply a matrix by a value

inline void smmult (matrixptr m, double s)
{
  for (int i=0;i<m->rows;i++)
    for (int j=0;j<m->cols;j++)
      *(m->block+i*m->cols+j)=*(m->block+i*m->cols+j)*s;
}


/// ---------------------------------------------------------------------------
/// Transpose matrix sm and put result in dm

int mtrans (matrixptr sm, matrixptr dm)
{
/*
  if (dm->rows!=sm->cols)
    etkError ("mtrans error - dest matrix rows must = source matrix cols");

  if (sm->rows!=dm->cols)
    etkError ("mtrans error - source matrix rows must = dest matrix cols");
*/
  for (int i=0;i<sm->rows;i++)
    for (int j=0;j<sm->cols;j++)
      {
      *(dm->block+j*dm->cols+i)=*(sm->block+i*sm->cols+j);
      }

  return(0);
}

/// ---------------------------------------------------------------------------
/// Find determinant of matrix m

inline double det (matrixptr m)
{
	double d=0;
	for (int i=0;i<m->cols;i++){
		double p1=1.0, p2=1.0;
		double p3=*(m->block+i);
		int k=i;
		for (int j=1;j<m->cols;j++){
			k=(k+1)%m->cols;
			p1*= *(m->block+j*m->cols+k);
			p2*= *(m->block+(m->cols-j)*m->cols+k);
		}
		p3*=(p1-p2);
		d+=p3;
	}
	return (d);
}

/// ---------------------------------------------------------------------------
/// Invert matrix sm and put result in dm
/*
int minv (matrixptr sm, matrixptr dm)
{
  if (sm->rows!=dm->rows||sm->cols!=dm->cols)
    etkError ("minv error - matrices must be same size");

  if (det(sm)==0.0)
    etkError ("minv error - matrix is singular");

  double* d = (double *)(malloc(sizeof(double)*sm->rows*(sm->cols+1)));

  if (d==(double *)NULL)
    etkError ("minv error - insufficient memory");

  for (int i=0;i<sm->rows;i++)
    {
      int nrow=i-1;
      int ncol=i-1;
      for (int j=0;j<sm->rows;j++)
        {
        nrow=(nrow+1)%sm->rows;
        if (j==0)
          *(d+j*(sm->cols+1)+sm->cols)=1;
        else
          *(d+j*(sm->cols+1)+sm->cols)=0;
        for (int k=0;k<sm->cols;k++)
          {
            ncol=(ncol+1)%sm->cols;
            *(d+j*(sm->cols+1)+k)=*(sm->block+nrow*sm->cols+ncol);
          }
    }

  if (nsolve(sm->rows,d))
  {
    free((char *)d);
    etkError ("minv error - cannot use nsolve"); // on row %u",i);
  }
  else
    {
      nrow=i-1;
      for (int j=0;j<sm->rows;j++)
      {
        nrow=(nrow+1)%sm->rows;
        *(dm->block+nrow*sm->cols+i)=*(d+j*(sm->cols+1)+sm->cols);
      }
    }
  }
  free((char *)d);
  return 0;
}
*/
/// ---------------------------------------------------------------------------
/// Solve equation in N unknowns

int nsolve (int rows, double* data)
{
  int i, k, cols = rows + 1;

  for (i = 0; i < rows; i++)
    {
      int j;
      for (j = i; j < rows && *(data+j*cols+j) == 0.0; j++);
      if (*(data+j*cols+j)==0.0)
        {
        etkError ("nsolve error - singular matrix");
        return 1;
        }
      if (j!=i)
        {
          for (k = 0; k < cols; k++)
            {
              double dtemp=*(data+i*cols+k);
              *(data+i*cols+k)=*(data+j*cols+k);
              *(data+j*cols+k)=dtemp;
            }
        }
      for (j = cols - 1; j >= 0; j--)
        {
          *(data+i*cols+j) /= *(data+i*cols+i);
        }

      for (j = i + 1; j < rows; j++)
        {
          for (int k=cols-1;k>=i;k--)
            *(data+j*cols+k)-=*(data+j*cols+i) * *(data+i*cols+k);
        }
    }
    for (i = rows - 2; i >= 0;i--)
      {
      	for (int j = cols - 2; j > i;j--)
          {
	    *(data+i*cols+cols-1)-= \
	    *(data+i*cols+j) * *(data+j*cols+cols-1);
	    *(data+i*cols+j)=0;
          }
      }
  return 0;
}

/// ---------------------------------------------------------------------------
/// Set matrix to the identity matrix

inline int mid (matrixptr m)
{
  for (int i=0;i<m->rows;i++){
      for (int j=0;j<m->cols;j++)
        *(m->block+i*m->cols+j)=0.0;
      *(m->block+i*m->cols+i)=1.0;
    }
  return 0;
}

/// ---------------------------------------------------------------------------
/// Set matrix to the identity matrix

void mzero (matrixptr m)
{
	int nsize = sizeof(double)*m->rows*m->rows;
    memset(m->block, 0, nsize);
}

/// ---------------------------------------------------------------------------

