//FILE: zsparse_matrix.cpp
#include "zsparse_matrix.h"
#include "lsolver/gmres.h"
#include "lsolver/bicgsq.h"
#include "lsolver/bicgstab.h"

CSparseMatrix33::CSparseMatrix33(const int n, const int dtype):
	CMemoryMgr()
{
	m_nRows = n;
	m_nElementCount = 0;
	m_nDataType = dtype;
	m_pRows = new CRowPointer[n];
	assert(m_pRows!=NULL);
	memset(m_pRows, 0, sizeof(CRowPointer)*n);
}


CSparseMatrix33::CSparseMatrix33(const CSparseMatrix33& A):
	CMemoryMgr()
{
	m_nRows = A.m_nRows;
	m_pRows = new CRowPointer[m_nRows];
	m_nDataType = A.m_nDataType;
	assert(m_pRows!=NULL);		
	for (int i=0; i<m_nRows; i++){
		CRowPointer &dstrow = m_pRows[i];
		CRowPointer &srcrow = A.m_pRows[i];
		for (int j=0; j<MAX_POINTERS_PER_ROW; j++){
			dstrow.m_pRowList[j]=_dupList(srcrow.m_pRowList[j]);
		}
	}
	m_nElementCount = A.m_nElementCount;
}


CSparseMatrix33::~CSparseMatrix33(void)
{
	SafeDeleteArray(m_pRows);
	Free();
}


void CSparseMatrix33::init(const int nRows)
{	//clear all buffers
	Free();

	//clear the col pointers
	m_nElementCount = 0;
	if (nRows>m_nRows){
		m_nRows = nRows;
		SafeDeleteArray(m_pRows);			
		m_pRows = new CRowPointer[nRows];
		assert(m_pRows!=NULL);
	}

	memset(m_pRows, 0, sizeof(CRowPointer)*m_nRows);
}


int CSparseMatrix33::getAllElementsInOneRow(const int rowid, int colID[], double val[])
{
	int c = 0;
	CRowPointer& row = m_pRows[rowid];
	if (m_nDataType==0){
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				val[c] = phead->m_data.x[0];
				colID[c] = phead->m_nColID;
				c++;
				phead = phead->m_pNext;
			}
		}
	}
	else{
		assert(0);
	}
	return c;
}


double CSparseMatrix33::getElementComponetMaxAbsValue(void)
{
	double v = 0;
	for (int j=0; j<m_nRows; j++){
		CRowPointer& row = m_pRows[j];
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				const double *x = &phead->m_data.x[0];
				for (int k=0; k<9; k++){
					const double y = fabs(x[k]);
					if (v<y) v=y;
				}
				phead = phead->m_pNext;
			}
		}
	}
	return v;
}


//test whether nodes (i,j) and (j,i) are symetric
bool CSparseMatrix33::_isSymetricElementPair(const int i, const int j) const
{
	bool r = true;
	ASSERT0(i>=0 && i<m_nRows);
	ASSERT0(j>=0 && j<m_nRows);
	CSparseMatrix33 & A = (CSparseMatrix33 &)(*this);
	const double3x3* eij = (const double3x3*)A.getMatrixElement(i, j);
	const double3x3* eji = (const double3x3*)A.getMatrixElement(j, i);			
	double3x3 eijt;
	if (eij==NULL){
		if (eji==NULL) return r;
		return false;
	}
	if (i==j){
		eijt = TransposeMatrix(*eij);
		eji = &eijt;
	}
	double3x3 dif = (*eij) - (*eji);
	double *x = dif.x;
	for (int i=0; i<9; i++){
		if (fabs(x[i])>1e-7){
			r = false;
			break;
		}
	}
	return r;
}


int CSparseMatrix33::_deleteZeroElmentsInRow(const int rowid, const double ZEROTOL)
{
	bool b;
	int c = 0;
	ASSERT0(rowid>=0 && rowid<m_nRows);
	CRowPointer &row = m_pRows[rowid];
	for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
		CLinkedListNode *pre = NULL;
		CLinkedListNode *phead = row.m_pRowList[i];
		while (phead!=NULL){
			const double *x = phead->m_data.x;
			if (m_nDataType) //stiffness element is 3X3 matrix
				b= (fabs(x[0])<ZEROTOL && fabs(x[4])<ZEROTOL && fabs(x[8])<ZEROTOL);
			else //stiffness element is only a double value
				b= fabs(x[0])<ZEROTOL;
			if (b){
				c++;
				if (pre==NULL)
					row.m_pRowList[i] = phead->m_pNext;
				else
					pre->m_pNext = phead->m_pNext;
			}
			else
				pre = phead;
			phead = phead->m_pNext;
		}
	}
	return c;
}

int CSparseMatrix33::deleteZeroElements(void)
{	
	const int BUFFSIZE = 500;
	int colID[BUFFSIZE];
	double val[BUFFSIZE];

	int c = 0;
	double sumval=0;
	for (int i=0; i<m_nRows>>3; i++){
		const int rowid = i<<3;
		int n = getAllElementsInOneRow(rowid, colID, val);
		if (n>BUFFSIZE){
			printf("BUFFER OVERFLOW!\n");
			assert(0);
		}
		for (int j=0; j<n; j++) sumval+=val[j];
		c+=n;
	}
	const double ZEROTOL = (fabs(sumval)/(c+1.0))*50E-10;

	c=0;
	for (int i=0; i<m_nRows; i++)
		c+=_deleteZeroElmentsInRow(i, ZEROTOL);
	const double percent = c*100.0/m_nElementCount;
	printf("Total %d, stiffness entries deleted: %d, percent %.1lf%%, ZEROTOL=%lf.\n", 
		m_nElementCount, c, percent, ZEROTOL);
	m_nElementCount-=c;
	return c;
}


/*
int SparseSolverGmres(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL)		//stopping criterion 
{
	const int m = 20;			//number of (inner) iterations until restart (only for gmres)
	const int n =A.getSize()*3; //dimension of the matrix 
    const int itr = gmres(m, n, A, (const double*)b, (double*)outvec, ETOL);
	return itr;
}

int SparseSolverBicgsq(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL)		//stopping criterion 
{
	const int n =A.getSize()*3; //dimension of the matrix 

    const int itr = bicgsq(n, A, (const double*)b, (double*)outvec, ETOL);
	return itr;
}

int SparseSolverBicgstab(
	CSparseMatrix33 &A,		//user-supplied matrix, of arbitrary type 
	const Vector3d* b,		//vector being solved, 
	Vector3d *outvec,		//before call: start vec for iter., after call: appro. solution of Ax=b
	const double& ETOL)		//stopping criterion 
{
	const int n =A.getSize()*3; //dimension of the matrix 

    const int itr = bicgstab(n, A, (const double*)b, (double*)outvec, ETOL);
	return itr;
}
*/

inline double _distMatrixElement(const double3x3 *a, const double3x3 *b)
{
	double dist = 0;
	if (a==NULL || b==NULL){
		if (a==NULL){
			if (b!=NULL)
				dist = InnerProd(*b, *b);
		}
		else{
			dist = InnerProd(*a, *a);
		}
	}
	else{
		double3x3 c = (*a) - (*b);
		dist = InnerProd(c, c);
	}
	return dist;
}


//Frobenius distance btw 2 matrices, they must be the same size
double Distance2(const CSparseMatrix33 &A, const CSparseMatrix33 &B)
{
	double dist = 0;
	const int wa = A.size();
	const int wb = A.size();
	if (wa!=wb) return dist;
	CSparseMatrix33 *pA = const_cast<CSparseMatrix33 *>(&A);
	CSparseMatrix33 *pB = const_cast<CSparseMatrix33 *>(&B);

	for (int i=0; i<wa; i++){
		for (int j=0; j<wa; j++){
			const double3x3 *a = pA->getMatrixElement(i, j);
			const double3x3 *b = pB->getMatrixElement(i, j);
			dist+=_distMatrixElement(a, b);
		}
	}
	return dist;
}


inline double _distMatrixElement(const double *a, const double *b)
{
	double dist = 0;
	if (a==NULL || b==NULL){
		if (a==NULL){
			if (b!=NULL)
				dist = (*b)*(*b);
		}
		else{
			dist = (*a) * (*a);
		}
	}
	else{
		double c = (*a) - (*b);
		dist = (c) * (c);
	}
	return dist;
}


double Distance2Diagonal(const CSparseMatrix33 &A, const CSparseMatrix33 &B)
{
	double dist = 0;
	const int wa = A.size();
	const int wb = A.size();
	if (wa!=wb) return dist;
	CSparseMatrix33 *pA = const_cast<CSparseMatrix33 *>(&A);
	CSparseMatrix33 *pB = const_cast<CSparseMatrix33 *>(&B);

	for (int i=0; i<wa; i++){
		double3x3 *a = pA->getMatrixElement(i, i);
		double3x3 *b = pB->getMatrixElement(i, i);
		double *xa = NULL;
		double *xb = NULL;
		if (a) xa = &a->x[4];
		if (b) xb = &b->x[4];
		dist+=_distMatrixElement(xa, xb);
	}
	return dist;
}


double Distance2DiagonalBlock(const CSparseMatrix33 &A, const CSparseMatrix33 &B)
{
	double dist = 0;
	const int wa = A.size();
	const int wb = A.size();
	if (wa!=wb) return dist;
	CSparseMatrix33 *pA = const_cast<CSparseMatrix33 *>(&A);
	CSparseMatrix33 *pB = const_cast<CSparseMatrix33 *>(&B);

	for (int i=0; i<wa; i++){
		double3x3 *a = pA->getMatrixElement(i, i);
		double3x3 *b = pB->getMatrixElement(i, i);
		dist += _distMatrixElement(a, b);
	}
	return dist;
}

//					0       1		2		3		4	  5
//(ax+by+c)^2 ==> a^2x^2 + b^2y^2 + c^2 + 2abxy + 2acx + 2bcy
inline Vector6d square_dist_symbolic(const Vector3d &v)
{
	const double a=v.x, b=v.y, c=v.z;
	const Vector6d r(a*a, b*b, c*c, 2*a*b, 2*a*c, 2*b*c);
	return r;
}

//f(l, m) = sum(L+M-K)
void BiQuadraticSymbolicDistance(
	    const int distType, 
		const CSparseMatrix33 &L, 
		const CSparseMatrix33 &M, 
		const CSparseMatrix33 &K, 
		Vector6d& cofficients)
{
	const int wa = L.size();
	const int wb = M.size();
	if (wa!=wb) return;
	Vector6d sum(0.0);
	for (int i=0; i<wa; i++){
		for (int j=0; j<wa; j++){
			if ((distType==1) && (i!=j)) continue;
			const double3x3* p = L.getMatrixElement(i, j);
			if (p){
				const double3x3 a = *p;
				const double3x3 b = *(M.getMatrixElement(i, j));
				const double3x3 c = *(K.getMatrixElement(i, j));
				for (int k=0; k<9; k++){
					const Vector3d v(a.x[k], b.x[k], -(c.x[k]));
					const Vector6d r = square_dist_symbolic(v);
					sum += r;
				}
			}
		}
	}
	cofficients = sum;
}


