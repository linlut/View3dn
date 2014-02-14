#include <stdio.h>
#include <math.h>
#include <set>
#include <algorithm>
#include "sparseMatrix.h"
using namespace std;

SparseMatrixOutline::SparseMatrixOutline(int n): n_(n)
{
  MakeEmpty();
}

SparseMatrixOutline::~SparseMatrixOutline()
{
  // deallocate individual row maps
  for(int i=0; i<n_; i++)
    entries_[i].clear();

  // deallocate vector entries_
  entries_.clear();
}

SparseMatrixOutline::SparseMatrixOutline(int n, double * diagonal): n_(n)
{
  MakeEmpty();

  pair<int,double> entry;

  for(int i=0; i<n_; i++)
  {
    entry.first = i;
    entry.second = diagonal[i];
    entries_[i].insert(entry);
  }
}

SparseMatrixOutline::SparseMatrixOutline(char * filename, int expand)
{
  if (expand <= 0)
  {
    printf("Error: invalid expand factor %d in SparseMatrixOutline constructor.\n", expand);
    throw 1;
  }
  
  FILE * inputMatrix = fopen(filename,"ra");
  if (!inputMatrix)
  {
    printf("Error: couldn't open input sparse matrix file %s.\n",filename);
    throw 2;
  }

  // read input size 
  int m,n;
  fscanf(inputMatrix,"%d\n%d\n",&m,&n);

  n_ = expand * m;

  printf("Loading matrix from %s... Size is %d x %d .\n",filename,n_,expand*n);fflush(NULL);

  MakeEmpty();

  char s[4096];
  while (fgets(s,4096,inputMatrix) != NULL)
  {
    int i1,j1;
    double x;
    sscanf(s, "%d %d %lf\n", &i1, &j1, &x);
    for(int e=0; e<expand; e++)
      AddEntry(expand * i1 + e, expand * j1 + e, x);
  }

  fclose(inputMatrix);
}

void SparseMatrixOutline::MakeEmpty()
{
  // allocate empty lists, one for very row
  map<int,double> emptyMap;
  for (int i=0; i<n_; i++)
    entries_.push_back(emptyMap);
}

void SparseMatrixOutline::AddEntry(int i, int j, double value)
{
  map<int,double>::iterator pos = entries_[i].find(j);
  if (pos != entries_[i].end())
    pos->second += value;
  else
  {
    pair<int,double> entry(j,value);
    entries_[i].insert(entry);
  }
}

void SparseMatrixOutline::AddBlock3x3Entry(int i, int j, double * matrix3x3)
{
	const int i3 =3*i;
	const int j3 =3*j;
	int cc = 0;
	for(int k=0; k<3; k++){
		for(int l=0; l<3; l++){
			AddEntry(i3+k,j3+l,matrix3x3[cc]);
			cc++;
	  }
	}
}

double SparseMatrixOutline::GetEntry(int i, int j)
{
  map<int,double>::iterator pos = entries_[i].find(j);
  if (pos != entries_[i].end())
    return (pos->second);
  else
    return 0;
}

int SparseMatrixOutline::GetNumColumns()
{
  int numColumns = -1;
  for(int i=0; i< n_; i++)
  {
    map<int,double>::iterator j1;
    // over all row entries 
    for(j1=entries_[i].begin(); j1 != entries_[i].end(); j1++) 
      if (j1->first > numColumns)
        numColumns = j1->first;
  }
  return numColumns + 1;
}

int SparseMatrixOutline::Save(char * filename)
{
  FILE * fout = fopen(filename,"wa");
  if (!fout)
    return 1;

  fprintf(fout,"%d\n%d\n",n_,GetNumColumns());
  for(int i=0; i< n_; i++)
  {
    map<int,double>::iterator j1;
    // over all row entries 
    for(j1=entries_[i].begin(); j1 != entries_[i].end(); ++j1) 
      fprintf(fout,"%d %d %.15f\n",i,j1->first,j1->second);

  }
  fclose(fout);

  return 0;
}

void SparseMatrixOutline::Print()
{
  for (int i=0; i<n_; i++)
  {
    for (int j=0; j<n_; j++)
      printf("%f ",GetEntry(i,j));
    printf("\n");
  }
}

int SparseMatrixOutline::GetNumEntries()
{
  int num = 0;
  for(int i=0; i<n_; i++)
    num += entries_[i].size();
  return num;
}

SparseMatrix::SparseMatrix(char * filename)
{
  SparseMatrixOutline sparseMatrixOutline(filename);
  InitFromOutline(&sparseMatrixOutline);
}

SparseMatrix::SparseMatrix(SparseMatrixOutline * sparseMatrixOutline)
{
  InitFromOutline(sparseMatrixOutline);
}

// construct matrix from the outline
void SparseMatrix::InitFromOutline(SparseMatrixOutline * sparseMatrixOutline)
{
  n_ = sparseMatrixOutline->Getn();
  Allocate();

  for(int i=0; i<n_; i++)
  {
    rowLength_[i] = sparseMatrixOutline->entries_[i].size();
    cindices_[i] = (int*) malloc (sizeof(int) * rowLength_[i]);
    centries_[i] = (double*) malloc (sizeof(double) * rowLength_[i]);

    map<int,double>::iterator pos;
    int j = 0;
    int prev = -1;
    for(pos = sparseMatrixOutline->entries_[i].begin(); pos != sparseMatrixOutline->entries_[i].end(); pos++)
    {
      cindices_[i][j] = pos->first;
      if (cindices_[i][j] <= prev)
        printf("Warning: entries not sorted in a row in a sparse matrix.\n");
      prev = cindices_[i][j];
      centries_[i][j] = pos->second;
      j++;
    }
  }
}

// allocator
void SparseMatrix::Allocate()
{
  // compact representation for fast matrix multiplications
  rowLength_ = (int*) malloc(sizeof(int) * n_);
  cindices_ = (int**) malloc(sizeof(int*) * n_);
  centries_ = (double**) malloc(sizeof(double*) * n_);
  diagonalIndices_ = NULL;
  subMatrixIndices_ = NULL;
  superMatrixIndices_ = NULL;
  superRows_ = NULL;
  transposeIndex_ = NULL;
}

// destructor
SparseMatrix::~SparseMatrix()
{
  for(int i=0; i<n_; i++)
  {
    free(cindices_[i]);
    free(centries_[i]);
  }

  if (subMatrixIndices_ != NULL)
  {
    for(int i=0; i<n_; i++)
    {
      free(subMatrixIndices_[i]);
    } 
    free(subMatrixIndices_);
    free(subMatrixIndexLengths_);
  }

  if (superRows_ != NULL)
  {
    for(int i=0; i<n_; i++)
      free(superMatrixIndices_[i]);
    free(superMatrixIndices_);
    free(superRows_);
  }

  free(rowLength_);
  free(cindices_);
  free(centries_);
  free(diagonalIndices_);
  FreeTranspositionIndices();
}

SparseMatrix::SparseMatrix(SparseMatrix & source) // copy constructor
{
  //printf("Copy constr:\n");fflush(NULL);
  n_ = source.Getn();

  // compact representation for fast matrix multiplications
  rowLength_ = (int*) malloc(sizeof(int) * n_);
  cindices_ = (int**) malloc(sizeof(int*) * n_);
  centries_ = (double**) malloc(sizeof(double*) * n_);

  diagonalIndices_ = NULL; // should also copy these
  subMatrixIndices_ = NULL;
  superRows_ = NULL;
  superMatrixIndices_ = NULL;
  transposeIndex_ = NULL;

  for(int i=0; i<n_; i++)
  {
    rowLength_[i] = source.rowLength_[i];
    cindices_[i] = (int*) malloc (sizeof(int) * rowLength_[i]);
    centries_[i] = (double*) malloc (sizeof(double) * rowLength_[i]);

    for(int j=0; j < rowLength_[i]; j++)
    {
      cindices_[i][j] = source.cindices_[i][j];
      centries_[i][j] = source.centries_[i][j];
    }
  }

  if (source.subMatrixIndices_ != NULL)
  {
    subMatrixIndices_ = (int**) malloc(sizeof(int*) * n_);
    subMatrixIndexLengths_ = (int*) malloc(sizeof(int) * n_);

    for(int i=0; i<n_; i++)
    {
      subMatrixIndexLengths_[i] = source.subMatrixIndexLengths_[i];
      subMatrixIndices_[i] = (int*) malloc(sizeof(int) * subMatrixIndexLengths_[i]);
      for(int j=0; j < subMatrixIndexLengths_[i]; j++)
      {
        subMatrixIndices_[i][j] = source.subMatrixIndices_[i][j];
      }
    }
  }

  if (source.superRows_ != NULL)
  {
    superRows_ = (int*) malloc(sizeof(int) * n_);
    superMatrixIndices_ = (int**) malloc(sizeof(int*) * n_);
    for(int i=0; i<n_; i++)
    {
      superRows_[i] = source.superRows_[i];
      superMatrixIndices_[i] = (int*) malloc(sizeof(int) * source.rowLength_[i]);
      for(int j=0; j < source.rowLength_[i]; j++)
      {
        superMatrixIndices_[i][j] = source.superMatrixIndices_[i][j];
      }
    }
  }

}

void SparseMatrix::MultiplyVector(double * input, double * result)
{
  for(int i=0; i<n_; i++)
  {
    result[i] = 0;
    for(int j=0; j < rowLength_[i]; j++)
    {
      result[i] += input[cindices_[i][j]] * centries_[i][j];
    }
  }
}

void SparseMatrix::MultiplyVectorAdd(double * input, double * result)
{
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j < rowLength_[i]; j++)
    {
      result[i] += input[cindices_[i][j]] * centries_[i][j];
    }
  }
}

double SparseMatrix::QuadraticForm(double * input)
{
  double result = 0;
 
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j < rowLength_[i]; j++)
    {
      int index = cindices_[i][j];
      if (index < i)
        continue;
      if (index == i)
        result += centries_[i][j] * input[i] * input[index];
      else
        result += 2.0 * centries_[i][j] * input[i] * input[index];
    }
  }
  
  return result;
} 

void SparseMatrix::NormalizeVector(double * vector)
{
  double norm = sqrt(QuadraticForm(vector));
  for(int i=0; i<n_; i++)
    vector[i] /= norm;
}

SparseMatrix & SparseMatrix::operator*=(const double alpha)
{
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] *= alpha;
  return *this;
}

SparseMatrix & SparseMatrix::operator+=(const SparseMatrix & mat2)
{   
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] += mat2.centries_[i][j];
  return *this;
}

SparseMatrix & SparseMatrix::operator-=(const SparseMatrix & mat2)
{  
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] -= mat2.centries_[i][j]; 
  return *this;
}

SparseMatrix & SparseMatrix::operator=(SparseMatrix & source)
{
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] = source.centries_[i][j];
  return *this;
}


void SparseMatrix::ScalarMultiply(const double alpha, SparseMatrix * dest)
{
  if (dest == NULL)
    dest = this;

  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      dest->centries_[i][j] = centries_[i][j] * alpha;
}

void SparseMatrix::ResetToZero()
{
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] = 0;
}

void SparseMatrix::Print()
{
  for (int i=0; i<n_; i++)
  {
    int index = 0;
    for(int j=0; j< rowLength_[i]; j++)
    {
      while (index < cindices_[i][j])
      {
        index++;
        printf("%f,",0.0);
      }
      printf("%f,",centries_[i][j]);
      index++;
    }

    while (index < n_)
    {
      index++;
      printf("%f,",0.0);
    }
    
    printf("\n");
  } 
}

// finds the position in row i of element with column index jAbsolute
int SparseMatrix::GetInverseIndex(int i, int jAbsolute)
{
  for(int j=0; j < rowLength_[i]; j++)
    if (cindices_[i][j] == jAbsolute)
      return j;

  return -1;
}


void SparseMatrix::BuildDiagonalIndices()
{
  diagonalIndices_ = (int*) malloc (sizeof(int) * n_);
  for(int i=0; i<n_; i++)
    diagonalIndices_[i] = GetInverseIndex(i,i);
}

void SparseMatrix::AddDiagonalMatrix(double * diagonal)
{
  if (diagonalIndices_ != NULL)
  {
    for(int i=0; i<n_; i++)
      centries_[i][diagonalIndices_[i]] += diagonal[i];
  }
  else
  {
    for(int i=0; i<n_; i++)
      for(int j=0; j<GetRowLength(i); j++)
      {
        if (GetColumnIndex(i, j) == i)
          centries_[i][j] += diagonal[i];
      }
  }
}

int SparseMatrix::GetNumEntries()
{
  int num = 0;
  for(int i=0; i<n_; i++)
    num += rowLength_[i];

  return num;
}

double SparseMatrix::SumEntries()
{
  double sum=0;
  for(int i=0; i<n_; i++)
    for(int j=0; j<rowLength_[i]; j++)
      sum += centries_[i][j];

  return sum;
}


void SparseMatrix::MakeLinearDataArray(double * data)
{
  int count=0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      data[count] = centries_[i][j];
      count++;
    }
  }   
}

void SparseMatrix::MakeLinearRowIndexArray(double * indices)
{
  int count=0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      indices[count] = i;
      count++;
    }
  }   
}

void SparseMatrix::MakeLinearColumnIndexArray(double * indices)
{
  int count=0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      indices[count] = cindices_[i][j];
      count++;
    }
  }   
}

void SparseMatrix::MakeLinearRowIndexArray(int * indices)
{
  int count=0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      indices[count] = i;
      count++;
    }
  }   
}

void SparseMatrix::MakeLinearColumnIndexArray(int * indices)
{
  int count=0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      indices[count] = cindices_[i][j];
      count++;
    }
  }   
}

void SparseMatrix::FreeTranspositionIndices()
{
  if (transposeIndex_ == NULL)
    return;

  for(int i=0; i<n_; i++)
    free(transposeIndex_[i]);
  free(transposeIndex_);

  transposeIndex_ = NULL;
}

void SparseMatrix::BuildTranspositionIndices()
{
  if (transposeIndex_ != NULL)
    return;

  transposeIndex_ = (int**) malloc (sizeof(int*) * n_);

  for(int i=0; i<n_; i++)
  {
    transposeIndex_[i] = (int*) malloc (sizeof(int) * rowLength_[i]);
    for(int j=0; j<rowLength_[i]; j++)
    {
      // absolute entry coordinates: (i,cindices_[i][j])
      int index = GetInverseIndex(cindices_[i][j],i);
      transposeIndex_[i][j] = index;
    }   
  }  
}

double SparseMatrix::SkewSymmetricCheck()
{
  double maxEntry = 0;  

  BuildTranspositionIndices();  

  for(int i=0; i<n_; i++)  
  {    
    for(int j=0; j<GetRowLength(i); j++)    
    {      
      double entry1 = GetEntry(i, j);      
      int tindex = TransposedIndex(i, j);      
      double entry2 = GetEntry(GetColumnIndex(i,j), tindex);      

      // entry1 + entry2 should be zero          
      if (fabs(entry1 + entry2) > maxEntry)
        maxEntry = fabs(entry1 + entry2);
    }  
  }  

  FreeTranspositionIndices();

  return maxEntry;
}

void SparseMatrix::SymmetrizeMatrix()
{
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      int jAbs = cindices_[i][j];

      if (jAbs >= i)
        break; 

      // copy elt (jAbs,i) into position (i,jAbs)
      centries_[i][j] = centries_[jAbs][TransposedIndex(i,j)];
    }
  }
}

double SparseMatrix::GetMaxAbsEntry()
{
  double max = 0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j<rowLength_[i]; j++)
    {
      double el = fabs(GetEntry(i,j));
      if (el > max)
        max = el;
    }
  }
  
  return max;
}

// solve M * x = b
// ASSUMES the sparse matrix is diagonal !!!
void SparseMatrix::DiagonalSolve(double * rhs)
{
  for(int i=0; i< n_; i++)
    rhs[i] /= centries_[i][0]; // the diagonal element
}

void SparseMatrix::BuildSuperMatrixIndices(int numFixedVertices, int * fixedVertices, SparseMatrix * constrainedMatrix)
{
  // build renumbering function
  constrainedMatrix->superRows_ = (int*) malloc (sizeof(int) * (n_-numFixedVertices));

  int numrows = 0;
  int row = 0;
                                                                                                                                                             
  for(int i=0; i<numFixedVertices; i++)
  {
    int index = fixedVertices[i];
    if ((index > n_) || (index < 1))
    {
      printf("Error: invalid index %d specified.\n",index);
      exit(1);
    }
    index--;
                                                                                                                                                             
    while (row<index)
    {
      constrainedMatrix->superRows_[numrows] = row;
      numrows++;
      row++;
    }
                                                                                                                                                             
    row++; // skip the deselected row
                                                                                                                                                             
    if (numrows > n_)
    {
      printf("Error: too many rows specified.\n");
      exit(1);
    }
  }
                                                                                                                                                             
  while (row < n_)
  {
    constrainedMatrix->superRows_[numrows] = row;
                                                                                                                                                             
    numrows++;
    row++;
                                                                                                                                                             
    if (numrows > n_)
    {
      printf("Error: too many rows specified.\n");
      exit(1);
    }
  }

  // now renumbering function has been built
  //printf("@1\n");fflush(NULL);

  // build row indices
  constrainedMatrix->superMatrixIndices_ = (int**) malloc (sizeof(int*) * (n_ - numFixedVertices)); 
  for(int i=0; i < n_ - numFixedVertices; i++)
  {
    constrainedMatrix->superMatrixIndices_[i] = (int*) malloc (sizeof(int) *  constrainedMatrix->rowLength_[i]);
    for(int j=0; j<constrainedMatrix->rowLength_[i]; j++)
    {
      int iConstrained = i;
      int jConstrainedAbsolute = constrainedMatrix->cindices_[iConstrained][j];
      int iSuper = constrainedMatrix->superRows_[iConstrained];
      int jSuperAbsolute = constrainedMatrix->superRows_[jConstrainedAbsolute];
      int jSuper = GetInverseIndex(iSuper, jSuperAbsolute);
      if (jSuper < 0)
      {
        printf("Error: failed to compute inverse index.\n");
        printf("Row: %d Absolute index: %d\n", constrainedMatrix->superRows_[i], jSuperAbsolute);
        fflush(NULL);
      }
      constrainedMatrix->superMatrixIndices_[i][j] = jSuper;
    }
  } 
}

void SparseMatrix::AssignSuperMatrix(SparseMatrix * superMatrix)
{
  for(int i=0; i<n_; i++)
  {
    double * row = superMatrix->centries_[superRows_[i]];
    int * indices = superMatrixIndices_[i];
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] = row[indices[j]];
  }
}

void SparseMatrix::BuildSubMatrixIndices(SparseMatrix & mat2)
{
  subMatrixIndices_ = (int**) malloc (sizeof(int*) * n_);
  subMatrixIndexLengths_ = (int*) malloc (sizeof(int*) * n_);

  for(int i=0; i<n_; i++)
  {
    subMatrixIndices_[i] = (int*) malloc (sizeof(int) * mat2.rowLength_[i]);
    subMatrixIndexLengths_[i] = mat2.rowLength_[i];
    int * indices = mat2.cindices_[i];
    for(int j=0; j < mat2.rowLength_[i]; j++)
    {
      // finds the position in row i of element with column index jAbsolute
      // int inverseIndex(int i, int jAbsolute);
      subMatrixIndices_[i][j] = GetInverseIndex(i,indices[j]);
    }
  }
}

SparseMatrix & SparseMatrix::AddSubMatrix(double factor, SparseMatrix & mat2)
{
  for(int i=0; i<n_; i++)
  {
    int * indices = subMatrixIndices_[i];
    for(int j=0; j < mat2.rowLength_[i]; j++)
      centries_[i][indices[j]] += factor * mat2.centries_[i][j];
  }

  return *this;
}

SparseMatrix & SparseMatrix::AddMatrix(const double factor, const SparseMatrix & mat2)
{
  for(int i=0; i<n_; i++)
    for(int j=0; j < rowLength_[i]; j++)
      centries_[i][j] += factor * mat2.centries_[i][j];

  return *this;
}

int SparseMatrix::GetNumLowerTriangleEntries()
{
  int num = 0;
  for(int i=0; i<n_; i++)
  {
    for(int j=0; j < rowLength_[i]; j++)
    {
      if (cindices_[i][j] <= i)
        num++;
    }
  }
  return num;
}

int SparseMatrix::GenerateNAGFormat(double * a, int * irow, int * icol, int * istr)
{
  int num = 0;
  for(int i=0; i<n_; i++)
  {
    istr[i] = num; // starting address of row i
    for(int j=0; j < rowLength_[i]; j++)
    {
      if (cindices_[i][j] <= i) // over lower triangle
      {
        a[num] = centries_[i][j];
        irow[num] = i+1; // NAG is 1-indexed
        icol[num] = cindices_[i][j]+1; // NAG is 1-indexed
        num++;
      }
    }
  }
  
  istr[n_] = num;

  return num;
} 

int SparseMatrix::Save(char * filename)
{
  FILE * fout = fopen(filename,"wa");
  if (!fout)
    return 1;

  fprintf(fout,"%d\n%d\n",n_,n_);
  for(int i=0; i< n_; i++)
  {
    for(int j=0; j < rowLength_[i]; j++)
    {
      int index = cindices_[i][j]; 
      double entry = centries_[i][j];
      fprintf(fout,"%d %d %.15G\n",i,index,entry);
    }
  }
  fclose(fout);

  return 0;
}

void SparseMatrix::RemoveRowColumn(int index)
{
  // remove row 'index'
  free(centries_[index]);
  free(cindices_[index]);

  for(int i=index; i<n_-1; i++)
  {
    centries_[i] = centries_[i+1];
    cindices_[i] = cindices_[i+1];
    rowLength_[i] = rowLength_[i+1];
  }

  // remove column 'index'
  for(int i=0; i<n_-1; i++)
  {
    // over all rows
    for(int j=0; j<rowLength_[i]; j++)
    {
      // seek for entry 'index'
      if (cindices_[i][j] == index) // found
      {
        // shift all elements ahead one step back
        for(int k=j; k<rowLength_[i]-1; k++)
        {
          cindices_[i][k] = cindices_[i][k+1];
          centries_[i][k] = centries_[i][k+1];
        } 
        rowLength_[i]--;
      }
    }

    // decrease indices for DOFs above index
    for(int j=0; j<rowLength_[i]; j++)
    {
      if(cindices_[i][j] > index)     
      {
        // decrease index
        cindices_[i][j]--;
      }
    }   
  }

  n_--;
}

void SparseMatrix::RemoveRowsColumns1Indexed(int numRemovedRowsColumns, int * removedRowsColumns1Indexed)
{
/* faster, but still bugy
  // build list of DOFs to stay
  set<int> retainedDOFs;
  for(int i=0; i<n_; i++)
    retainedDOFs.insert(i);

  set<int> removedDOFs;
  for(int i=0; i<numRemovedRowsColumns; i++)
  {
    retainedDOFs.erase(removedRowsColumns1Indexed[i] - 1);
    removedDOFs.insert(removedRowsColumns1Indexed[i] - 1);
  }

  map<int,int> relabeling;
  int count = 0;
  for(set<int> :: iterator iter = retainedDOFs.begin(); iter != retainedDOFs.end(); iter++)
  {
    relabeling.insert(make_pair(*iter, count));
    count++;
  }

  set<int> :: iterator iter;
  int newIndex = 0;
  for(iter = retainedDOFs.begin(); iter != retainedDOFs.end(); iter++)
  {
    int originalIndex = *iter;

    // erase the removed entries in the current row
    map<int,double> row;
    for(int i=0; i<rowLength_[originalIndex]; i++)
    {
      int oldName = cindices_[originalIndex][i];
      if (removedDOFs.find(oldName) != removedDOFs.end())
      { 
        int newName = relabeling.find(oldName)->second;
        row.insert(make_pair(newName, centries_[originalIndex][i]));
      }
    }
    // row now contains the new row

    // reallocate pointers
    cindices_[newIndex] = (int*) realloc(cindices_[newIndex], sizeof(int) * row.size());
    centries_[newIndex] = (double*) realloc(centries_[newIndex], sizeof(double) * row.size());
    rowLength_[newIndex] = row.size();

    // insert entries back into the row
    map<int,double> :: iterator iter;
    int i=0;
    for(iter = row.begin(); iter != row.end(); iter++)
    {
      cindices_[newIndex][i] = iter->first;
      centries_[newIndex][i] = iter->second;
      i++;
    }

    newIndex++; 
  }

  // nuke old data
  for(int i=newIndex; i<n_; i++)
  {
    free(centries_[i]);
    free(cindices_[i]);
  }
  
  n_ -= numRemovedRowsColumns;
  centries_ = (double**) realloc(centries_, sizeof(double*) * n_);
  cindices_ = (int**) realloc(cindices_, sizeof(double*) * n_);
  rowLength_ = (int*) realloc(rowLength_, sizeof(int) * n_);
*/

  // old (slower) implementation:
  for(int i=0; i<numRemovedRowsColumns; i++)
    RemoveRowColumn(removedRowsColumns1Indexed[i]-1-i);
}


double SparseMatrix::GetInfinityNorm()
{
  double norm = 0.0;

  for(int i=0; i< n_; i++)
  {
    double absRowSum = 0;

    for(int j=0; j<rowLength_[i]; j++)
    {
      absRowSum += fabs(centries_[i][j]);
     }

    if (absRowSum > norm)
      norm = absRowSum;
  }

  return norm;
}

void SparseMatrix::CheckLinearSystemSolution(double * x, double * b)
{
  double * buffer = (double*) malloc (sizeof(double) * n_);

  MultiplyVector(x,buffer);

  double inftyNorm = 0;
  double inftyNorm_b = 0;
  for(int i=0; i<n_; i++)
  {
    if (fabs(buffer[i] - b[i]) > inftyNorm)
      inftyNorm = fabs(buffer[i] - b[i]);

    if (fabs(b[i]) > inftyNorm_b)
      inftyNorm_b = fabs(b[i]);
  }

  printf("Infinity residual norm ||Ax-b|| is %G.\n", inftyNorm);
  printf("Relative infinity residual norm ||Ax-b||/||b|| is %G.\n", inftyNorm / inftyNorm_b);

  free(buffer);
}

