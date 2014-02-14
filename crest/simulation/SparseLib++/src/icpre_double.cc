/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*             ********   ***                                 SparseLib++    */
/*          *******  **  ***       ***      ***                              */
/*           *****      ***     ******** ********                            */
/*            *****    ***     ******** ********              R. Pozo        */
/*       **  *******  ***   **   ***      ***                 K. Remington   */
/*        ********   ********                                 A. Lumsdaine   */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*                                                                           */
/*                                                                           */
/*                     SparseLib++ : Sparse Matrix Library                   */
/*                                                                           */
/*               National Institute of Standards and Technology              */
/*                        University of Notre Dame                           */
/*              Authors: R. Pozo, K. Remington, A. Lumsdaine                 */
/*                                                                           */
/*                                 NOTICE                                    */
/*                                                                           */
/* Permission to use, copy, modify, and distribute this software and         */
/* its documentation for any purpose and without fee is hereby granted       */
/* provided that the above notice appear in all copies and supporting        */
/* documentation.                                                            */
/*                                                                           */
/* Neither the Institutions (National Institute of Standards and Technology, */
/* University of Notre Dame) nor the Authors make any representations about  */
/* the suitability of this software for any purpose.  This software is       */
/* provided ``as is'' without expressed or implied warranty.                 */
/*                                                                           */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "icpre_double.h"
#include "qsort_double.h"
#include "qsort_int.h"
#include "spblas.h"

static void
ICFactor(const VECTOR_int &ia, const VECTOR_int &ja, VECTOR_double &sa);


ICPreconditioner_double::ICPreconditioner_double(const CompCol_Mat_double &A)
  : val_(0), pntr_(A.dim(1)+1), indx_(0), nz_(0)
{
	dim_[0] = A.dim(0);
	dim_[1] = A.dim(1);
	int i, j, k;

	for (k = 0; k < dim_[1]; k++)
		for (j = A.col_ptr(k); j < A.col_ptr(k+1); j++)
			if (A.row_ind(j) >= k) nz_++;
	val_.newsize(nz_);
	indx_.newsize(nz_);

	//Copy just triangular part
	pntr_(0) = 0;
	for (k = 0; k < dim_[1]; k++) {
		pntr_(k+1) = pntr_(k);
		for (j = A.col_ptr(k); j < A.col_ptr(k+1); j++) {
			if (A.row_ind(j) >= k) {
				i = pntr_(k+1)++;
				val_(i) = A.val(j);
				indx_(i) = A.row_ind(j);
			}
		}
	}
	for (i = 0; i < dim_[1]; i++)
		QSort(indx_, val_, pntr_[i], pntr_[i+1] - pntr_[i]);

	//Find diagonal element to verify property
	for (i = 0; i < dim_[1]; i++)
		if (indx_[pntr_(i)] != i) {
			std::cerr << "IC Preconditioner: diagonal not found!" << i << "\n";
			exit(1);
		}
	ICFactor(pntr_, indx_, val_);
}


ICPreconditioner_double::ICPreconditioner_double(const CompRow_Mat_double &A)
  : val_(0), pntr_(A.dim(0)+1), indx_(0), nz_(0)
{
	dim_[0] = A.dim(0);
	dim_[1] = A.dim(1);
	int i, j, k;
	for (k = 0; k < dim_[0]; k++){
		for (j = A.row_ptr(k); j < A.row_ptr(k+1); j++)
			if (A.col_ind(j) >= k) nz_++;
	}
	val_.newsize(nz_);
	indx_.newsize(nz_);

	//Copy just triangular part (including diagonal)
	pntr_(0) = 0;
	for (k = 0; k < dim_[0]; k++) {
		pntr_(k+1) = pntr_(k);
		for (j = A.row_ptr(k); j < A.row_ptr(k+1); j++) {
			if (A.col_ind(j) >= k) {
				i = pntr_(k+1)++;
				val_(i) = A.val(j);
				indx_(i) = A.col_ind(j);
			}
		}
	}
	for (i = 0; i < dim_[0]; i++)
		QSort(indx_, val_, pntr_[i], pntr_[i+1] - pntr_[i]);

	//Find diagonal element to verify property
	for (i = 0; i < dim_[0]; i++){
		const int pos = pntr_(i);
		if (indx_[pos] != i) {
			std::cerr << "IC Preconditioner: diagonal not found!" << i << "\n";
			exit(1);
		}
	}
	ICFactor(pntr_, indx_, val_);
}


VECTOR_double
ICPreconditioner_double::solve(const VECTOR_double &x) const
{
  VECTOR_double y(x);

  ICSolve(pntr_, indx_, val_, y);

  return y;
}


VECTOR_double
ICPreconditioner_double::trans_solve(const VECTOR_double &x) const
{
  VECTOR_double y(x);

  ICSolve(pntr_, indx_, val_, y);

  return y;
}


static void 
ICFactor(const VECTOR_int &pntr, const VECTOR_int &indx, VECTOR_double &val)
{
	const double delta = 0.225;
	const double alpha = 1.0 - delta;
	const int n = pntr.size() - 1;
	const int n1 = n - 1;
	for (int k=0; k<n1; k++) {
		const int d = pntr[k];
		const int d1 = pntr[k+1];
		//printf("%d: ", k);
		//for (int i = d; i < d1; i++) printf("%d-%.3lf, ", indx[i], val[i]);
		//printf("\n");
		const double vald = val[d];
		val[d] = sqrt(vald) + delta*vald;
		const double zz1 = 1.0/val[d];			
		for (int i = d + 1; i < d1; i++)
			val[i] *= zz1;
		for (int i = d + 1; i < d1; i++) {
			const double z = val[i];
			const int h = indx[i];
			int g = i;
			for (int j = pntr[h]; j < pntr[h+1]; j++){
				for (; g < pntr[k+1] && indx[g+1] <= indx[j]; g++){
					if (indx[g] == indx[j])
						val[j] -= z * val[g];
					else{
						const int ii = indx[j];
						const int posii = pntr[ii];
						val[posii] -= (alpha*z)*val[g];
					}
				}
			}
		}
	}
	const int d = pntr[n1];
	val[d] = sqrt(val[d]) + delta*val[d];
}


void ICSolve(const VECTOR_int &pntr, const VECTOR_int &indx, 
    const VECTOR_double &val, VECTOR_double &dest)
{
  int M = dest.size();
  VECTOR_double work(M);
  int descra[9];
  descra[0] = 0;

  // lower diag
  descra[1] = 1;
  descra[2] = 0;
  F77NAME(dcscsm) (0, M, 1, 1, NULL, 1.0,
           descra, &val(0), &indx(0), &pntr(0),
           &dest(0), M, 0.0, &dest(1), M,
           &work(0), M);
  // lower diag transpose
  F77NAME(dcscsm) (1, M, 1, 1, NULL, 1.0,
           descra, &val(0), &indx(0), &pntr(0),
           &dest(0), M, 0.0, &dest(1), M,
           &work(0), M);
}
