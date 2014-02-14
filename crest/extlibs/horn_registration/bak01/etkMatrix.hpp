
/*! \file etkMatrix.hpp
    \brief Matrix structure and operations.
    (Original author:  Nigel Salt)
 */

#ifndef etkMatrix_hpp
#define etkMatrix_hpp 1

  /*!  
   *  \brief General matrix structure
   *  
   *  E.G.
   *  double b4x4A[4][4]=
   *  {
   *    6,1,6,6,
   *    1,6,6,0,
   *    0,3,2,1,
   *    8,6,1,9
   *   };
   * matrix m4x4A={4,4,&b4x4A[0][0]};
   */
  typedef struct
  {
    /// Number of rows
    int rows;
    /// Number of columns
    int cols;
    /// Data (line1, line2, ...)
    double *block;
  } matrix,*matrixptr;


  /// Print out a matrix
  void mprint (matrixptr);

  /// Scalar multiply a matrix by a value
  void smmult (matrixptr, double);

  /// Add matrix m1 to m2 giving dm
  int madd (matrixptr m1, matrixptr m2, matrixptr dm);

  /// Multiply matrix m1 by m2 giving dm
  inline int mmult (matrixptr m1, matrixptr m2, matrixptr dm)
  {
	for (int i=0; i<m1->rows; i++)
		for (int j=0; j<m2->cols; j++){
			double cellval = 0.0;
			for (int k=0; k<m1->cols; k++){
				cellval+=*(m1->block+i*(m1->cols)+k) * (*(m2->block+k*(m2->cols)+j));
			}
			*(dm->block+i*dm->cols+j)=cellval;
		}
	return(0);
  }

  /// Copy matrix sm to dm
  int mcopy(matrixptr sm,matrixptr dm);

  /// Transpose matrix sm and put result in dm
  int mtrans(matrixptr sm,matrixptr dm);

  /// Find determinant of matrix m
  double det(matrixptr m);

  /// Invert matrix sm and put result in dm
  int minv(matrixptr sm,matrixptr dm);

  /// Solve equation in N unknowns
  int nsolve(int rows,double *data);

  /// Set matrix to the identity matrix
  int mid(matrixptr);

  /// Set all cells of matrix to Zero
  void mzero(matrixptr);

#endif
