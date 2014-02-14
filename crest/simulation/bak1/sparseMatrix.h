/*

* Copyright (c) 2008, Carnegie Mellon University, Jernej Barbic, Doug L. James
* All rights reserved.
*
* Code author: Jernej Barbic
* Research: Jernej Barbic, Doug L. James
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Carnegie Mellon University, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Carnegie Mellon University, Jernej Barbic, 
* Doug L. James, ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Carnegie Mellon University, Jernej Barbic, or
  Doug L. James BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  This class implements sparse matrices (stores a list of indices of non-zero entries, 
  together with non-zero double precision entries), with the common algebraic 
  operations (incremental construction, addition, mtx-vec multiplication, 
  load/save to disk, row-column deletion, etc.) 

  Also included is a rudimentary Conjugate Gradient linear system solver 
  (for positive-definite large sparse symmetric matrices, without preconditioning). 
  The CG Solver was implemented directly by following Jonathan Shewchuk's 
  "An Introduction to the Conjugate Gradient Method Without the Agonizing Pain":
  http://www.cs.cmu.edu/~jrs/jrspapers.html#cg

  The matrices can be rectangular; they need not be necessarily square. Data is 
  stored row-wise. Each row is a list of non-zero locations, together with values 
  at those locations. All quantities (rows, columns, etc.) in this class are 0-indexed.

  There are two classes: SparseMatrixOutline and SparseMatrix.

  The SparseMatrixOutline class should be used to construct the non-zero 
  locations in the matrix, and (optionally) assign values to them.
  You should then transform it into a SparseMatrix class, via SparseMatrix's
  class constructor. The SparseMatrix class has all the computational routines,
  but you can only add new non-zero entries to SparseMatrixOutline, not
  to SparseMatrix.  The reason for this separation is that SparseMatrixOutline 
  does not know the number of matrix entries ahead of time, so it uses STL's 
  map to store the matrix data. In the SparseMatrix class, however, the number 
  of sparse entries and their locations are fixed, so all operations can use 
  known-length C arrays, which is faster.
  So: you should first create an instance of SparseMatrixOutline, then create 
  an instance of SparseMatrix by passing the SparseMatrixOutline object to 
  SparseMatrix's constructor.
  If your matrix is a text file on disk, you can load it to SparseMatrixOutline, 
  or directly load it into SparseMatrix (which will still proceed via 
  SparseMatrixOutline under the hood).

  The text disk file format is as follows:
  <number of matrix rows>
  <number of matrix columns>
  one or more data lines, each giving one matrix entry, in the format:
  <row index> <column index> <data value> 
  (indices are 0-indexed)

  Example:
  
    [0 17 -1 0]
  A=[0  5  0 0]
    [3  8  6 0]  

  would be given as:

  3
  4
  0 1 17 
  0 2 -1
  1 1 5
  2 0 3
  2 1 8
  2 2 6

*/

/*
  Version: 1.0
*/

#ifndef _SPARSE_MATRIX_H_
#define _SPARSE_MATRIX_H_

#ifdef WIN32
  #pragma warning(disable: 4786)
#endif

#include <vector>
#include <map>

class SparseMatrixOutline
{
public:
  // makes an empty sparse matrix with n rows
  SparseMatrixOutline(int n);
  ~SparseMatrixOutline();

  // makes a diagonal n x n sparse matrix; diagonal is a vector of n numbers
  SparseMatrixOutline(int n, double * diagonal);

  // loads the sparse matrix from a text file
  // if expand is greater than 1, the routine also expands each element into a diagonal block of size expand x expand... 
  //   (expand option is useful for loading the mass matrix in structural mechanics (with expand=3 in 3D))
  SparseMatrixOutline(char * filename, int expand=1); 

  // save matrix to a text file
  int Save(char * filename);

  // add entry at location (i,j) in the matrix
  void AddEntry(int i, int j, double value);
  void AddBlock3x3Entry(int i, int j, double * matrix3x3); // matrix3x3 should be given in row-major order

  inline int Getn() { return n_; } // get number of rows
  inline int GetNumRows() { return n_; } // get number of rows
  int GetNumColumns(); // get the number of columns (i.e., search for max column index)
  int GetNumEntries(); // get total number of non-zero matrix elements
  double GetEntry(int i, int j); // returns the matrix entry at location (i,j) in the matrix (or zero if entry has not been assigned)
  void Print();

  // low-level routine which is rarely used
  inline const std::map<int,double> & GetRow(int i) const { return entries_[i]; }
  friend class SparseMatrix;

protected:
  int n_;
  std::vector< std::map<int,double> > entries_;

  void MakeEmpty();
};

class SparseMatrix
{
public:

  SparseMatrix(char * filename); // load from text file (same text file format as SparseMatrixOutline)
  SparseMatrix(SparseMatrixOutline * sparseMatrixOutline); // create it from the outline
  SparseMatrix(SparseMatrix & source); // copy constructor
  ~SparseMatrix();

  int Save(char * filename); // save matrix to a disk text file 

  // set/add value to j-th non-zero element in row 'row' (not to matrix element at (row,j) !)
  inline void SetEntry(int row, int j, double value) { centries_[row][j] = value; }
  inline void AddEntry(int row, int j, double value) { centries_[row][j] += value; }
  void ResetToZero(); // reset all entries to zero

  inline int Getn() { return n_; } // get the number of rows
  inline int GetRowLength(int row) { return rowLength_[row]; }
  // return the j-th non-zero element in row i (not matrix element at (row, j) !)
  inline double GetEntry(int row, int j) { return centries_[row][j]; }
  // returns the column index of the j-th non-zero element in row 'row'
  inline int GetColumnIndex(int row, int j) { return cindices_[row][j]; } 
  // finds the position (in row 'row') of element with column index jAbsolute
  // returns -1 if column not found
  int GetInverseIndex(int row, int jAbsolute);

  int GetNumEntries(); // returns the total number of non-zero entries
  double SumEntries(); // returns the sum of all matrix entries
  double GetMaxAbsEntry(); // max abs value of a matrix entry
  double GetInfinityNorm(); // matrix infinity norm
  void Print(); // prints the matrix out to standard output

  // matrix algebra (all involved matrices must have the same topology)
  SparseMatrix & operator=(SparseMatrix & source);
  SparseMatrix & operator*=(const double alpha);
  SparseMatrix & operator+=(const SparseMatrix & mat2);
  SparseMatrix & operator-=(const SparseMatrix & mat2);
  SparseMatrix & AddMatrix(const double factor, const SparseMatrix & mat2); // M += factor * mat2
  void ScalarMultiply(const double alpha, SparseMatrix * dest=NULL); // dest := alpha * dest (if dest=NULL, operation is applied to this object)

  // multiplies the sparse matrix with the given vector
  void MultiplyVector(double * input, double * result);
  void MultiplyVectorAdd(double * input, double * result);

  // computes <M*input, input> (assumes symmetric M)
  double QuadraticForm(double * input);
  // normalizes vector in the M-norm: vector := vector / sqrt(<M*vector, vector>)  (assumes symmetric M)
  void NormalizeVector(double * vector);

  // writes all entries into the space provided by 'data'
  // space must be pre-allocated
  // data is written row after row, and by non-zero columns within each row
  void MakeLinearDataArray(double * data);
  // writes row indices of non-zero entries into array "indices"
  // same order as for data
  void MakeLinearRowIndexArray(int * indices);
  // indices in this function version are double to ensure compatibility with Matlab
  void MakeLinearRowIndexArray(double * indices);
  // writes column indices
  void MakeLinearColumnIndexArray(int * indices);
  void MakeLinearColumnIndexArray(double * indices);

  // removes row and column 'index' from the matrix
  void RemoveRowColumn(int index); // 0-indexed
  void RemoveRowsColumns1Indexed(int numRowColumns, int * rowColumns1Indexed); // 1-indexed

  // the transposition subsystem
  void BuildTranspositionIndices();
  void FreeTranspositionIndices();
  // returns the list position of transposed element (row, list position j)
  inline int TransposedIndex(int row, int j) { return transposeIndex_[row][j]; }

  // checks if the matrix is skew-symmetric
  // the matrix must have symmetric topology
  // returns max ( abs ( A^T + A ) ) = || A^T + A ||_{\infty}
  double SkewSymmetricCheck();
  // makes matrix symmetric by copying upper triangle + diagonal into the lower triangle
  // the matrix must have symmetric topology
  void SymmetrizeMatrix();

  // builds the positions of diagonal elements in each row
  // this routine will accelerate subsequent AddDiagonalMatrix routine calls, but is not necessary for AddDiagonalMatrix
  void BuildDiagonalIndices();
  void AddDiagonalMatrix(double * diagonal);

  // add a matrix to the current matrix, whose elements are a subset of the elements of the current matrix
  // call this once to establish the correspondence
  void BuildSubMatrixIndices(SparseMatrix & mat2);
  // += factor * mat2
  SparseMatrix & AddSubMatrix(double factor, SparseMatrix & mat2);

  // copy data from a matrix into a submatrix obtained by a previous call to RemoveRowColumns
  // call this once to inialize:
  void BuildSuperMatrixIndices(int numFixedVertices, int * fixedVertices, SparseMatrix * constrainedMatrix);
  // call this to quickly assign the values at the appropriate places in the submatrix:
  void AssignSuperMatrix(SparseMatrix * superMatrix);

  // diagonal solve M * x = b
  // ASSUMES the sparse matrix is diagonal !
  // result is overwritten into rhs
  // (to solve non-diagonal linear systems, you need to an external library; or you can use the CGSolver class included with this sparse matrix package)
  void DiagonalSolve(double * rhs);

  // returns the total number of non-zero entries in the lower triangle (including diagonal)
  int GetNumLowerTriangleEntries();
  // converts matrix to format for NAG library
  int GenerateNAGFormat(double * a,int * irow,int * icol, int * istr);
  void CheckLinearSystemSolution(double * x, double * b);

  // below are low-level routines which are rarely used
  inline double ** GetDataHandle() { return centries_; }
  inline double * GetRowHandle(int row) { return centries_[row]; }

protected:

  // compact representation for fast matrix multiplications
  int n_;
  int * rowLength_;
  int ** cindices_;
  double ** centries_;

  int * diagonalIndices_;
  int ** transposeIndex_;

  int ** subMatrixIndices_;
  int * subMatrixIndexLengths_;

  int ** superMatrixIndices_;
  int * superRows_;

  void InitFromOutline(SparseMatrixOutline * sparseMatrixOutline);
  void Allocate();
};

#endif

