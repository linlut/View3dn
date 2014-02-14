//FILE: zsparse_matrix.h
//A sparse matrix definition, where each element is a 3x3 submatrix,
//It is used in the implicit ODE solver: backward Euler method.

#ifndef _INC_ZSPARSE_MATRIX_ZN2008_H
#define _INC_ZSPARSE_MATRIX_ZN2008_H

#include <stdio.h>
#include <memory.h>
#include <memmgr.h>
#include <matrix3x3.h>
#include <vector6d.h>


//To speed up the element search, each row of the sparse matrix is subdivied into 
//8-way buckets, which is identified by the last 3 bits of the column index
#define MAX_POINTERS_PER_ROW	8
#define COL_POINTER_MASK		7



class CSparseMatrix33: public CMemoryMgr
{
protected:
	class CLinkedListNode
	{
	public:
		double3x3 m_data;
		int m_nColID;
		CLinkedListNode *m_pNext;
	public:
		CLinkedListNode(void)
		{
			m_pNext = NULL;
			m_nColID = 0;
			m_data.setZeroMatrix();
		}
	};

	class CRowPointer
	{
	public:
		CLinkedListNode *m_pRowList[MAX_POINTERS_PER_ROW];
	};

protected:
	CRowPointer *m_pRows;
	int m_nElementCount;
	int m_nRows;
	int m_nDataType;		//0: double , 1: double3x3 matrix
	
	inline static int _listSize(const CLinkedListNode* lst)
	{
		int n = 0;
		const CLinkedListNode* p = lst;
		while (p!=NULL){
			n++; p=p->m_pNext;
		}
		return n;
	}

	inline CLinkedListNode* _dupList(const CLinkedListNode* lst)
	{
		CLinkedListNode *phead = NULL;
		const CLinkedListNode *p2 = lst;
		
		//Alloc space first
		while (p2!=NULL){
			CLinkedListNode *pnode = (CLinkedListNode *)this->Malloc(sizeof(CLinkedListNode));
			pnode->m_data = p2->m_data;
			pnode->m_nColID = p2->m_nColID;
			p2 = p2->m_pNext;
			pnode->m_pNext = phead;
			phead = pnode;
		}
		return phead;
	}

	inline void _appendToMatrix(const int i, const int j, CLinkedListNode *pnewnode)
	{
		ASSERT0(i>=0 && i<m_nRows);
		ASSERT0(j>=0 && j<m_nRows);
		CRowPointer &row = m_pRows[i];
		const int jmasked = j & COL_POINTER_MASK;
		CLinkedListNode* &phead = row.m_pRowList[jmasked];
		pnewnode->m_pNext = phead;
		phead = pnewnode;
	}

	inline void _scaleRowWithVector(const int rowid, const Vector3d& s)
	{
		ASSERT0(rowid>=0 && rowid<m_nRows);
		CRowPointer &row = m_pRows[rowid];
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				double *x = phead->m_data.x;
				x[0]*=s.x, x[1]*=s.x, x[2]*=s.x;
				x[3]*=s.y, x[4]*=s.y, x[5]*=s.y;
				x[6]*=s.z, x[7]*=s.z, x[8]*=s.z;
				phead = phead->m_pNext;
			}
		}
	}

	inline void _multiplyRowWithVectorMatrix(const int rowid, const Vector3d *vin, Vector3d &vout) const
	{
		ASSERT0(rowid>=0 && rowid<m_nRows);
		vout.x = vout.y = vout.z = 0;
		CRowPointer &row = m_pRows[rowid];
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			const CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				vout+= phead->m_data * vin[phead->m_nColID];
				phead = phead->m_pNext;
			}
		}
	}

	inline void _multiplyRowWithVectorDouble(const int rowid, const Vector3d *vin, Vector3d &vout) const
	{
		ASSERT0(rowid>=0 && rowid<m_nRows);
		vout.x = vout.y = vout.z = 0;
		CRowPointer &row = m_pRows[rowid];
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			const CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				vout += phead->m_data.x[0] * vin[phead->m_nColID];
				phead = phead->m_pNext;
			}
		}
	}

	inline void _multiplyRowWithVectorMatrix(const int rowid, const Vector3d *vin, const int vin_stride, Vector3d &vout) const
	{
		vout.x = vout.y = vout.z = 0;
		CRowPointer &row = m_pRows[rowid];
		const char * psrc = (const char *)vin;
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			const CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				const Vector3d *pval = (const Vector3d*)(psrc + phead->m_nColID*vin_stride);
				vout += phead->m_data * (*pval);
				phead = phead->m_pNext;
			}
		}
	}


	inline void _multiplyRowWithVectorDouble(const int rowid, const Vector3d *vin, const int vin_stride, Vector3d &vout) const
	{
		vout.x = vout.y = vout.z = 0;
		CRowPointer &row = m_pRows[rowid];
		const char * psrc = (const char *)vin;
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			const CLinkedListNode *phead = row.m_pRowList[i];
			while (phead!=NULL){
				const Vector3d *pval = (const Vector3d*)(psrc + phead->m_nColID*vin_stride);
				vout += phead->m_data.x[0] * (*pval);
				phead = phead->m_pNext;
			}
		}
	}

	//ra = ra + rb;
	inline void _addRowWithAnotherRow(CRowPointer* ra, const CRowPointer* rb) 
	{
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			CLinkedListNode *pdst = ra->m_pRowList[i];
			const CLinkedListNode *psrc = rb->m_pRowList[i];
			while (pdst!=NULL){
				pdst->m_data += psrc->m_data;
				psrc = psrc->m_pNext;
				pdst = pdst->m_pNext;
			}
		}
	}

	//e(i,j) <= 0
	inline void _setRowElementsValue(CRowPointer *ra, const double3x3& zero) 
	{
		for (int i=0; i<MAX_POINTERS_PER_ROW; i++){
			CLinkedListNode *pdst = ra->m_pRowList[i];
			while (pdst!=NULL){
				pdst->m_data = zero;
				pdst = pdst->m_pNext;
			}
		}
	}

	inline double3x3* _getMatrixElement(const int i, const int j)
	{
		ASSERT0(i>=0 && i<m_nRows);
		ASSERT0(j>=0 && j<m_nRows);
		CRowPointer &row = m_pRows[i];
		const int jmasked = j & COL_POINTER_MASK;
		CLinkedListNode *phead = row.m_pRowList[jmasked];
		while (phead!=NULL){
			if (phead->m_nColID == j)
				return &phead->m_data;
			phead = phead->m_pNext;
		}
		return NULL;
	}

	//test whether nodes (i,j) and (j,i) are symetric
	bool _isSymetricElementPair(const int i, const int j) const;

	int _deleteZeroElmentsInRow(const int rowid, const double ZEROTOL);

public:
	void init(const int n);

	CSparseMatrix33(const int n, const int dtype);

	CSparseMatrix33(const CSparseMatrix33& A);

	virtual ~CSparseMatrix33(void);

	inline int size(void) const
	{
		return m_nRows;
	}

	//clear the matrix 
	inline void clear(void)
	{
		Free();
		m_nElementCount = 0;
		for (int j=0; j<m_nRows; ++j){
			CRowPointer &row = m_pRows[j];
			for (int i=0; i<MAX_POINTERS_PER_ROW; ++i)
				row.m_pRowList[i] = NULL;
		}
	}

	//set the matrix elements with zero
	inline void setZero(void)
	{
		const double3x3 zero(0.0); 
		for (int i=0; i<m_nRows; ++i){
			CRowPointer* ra = &m_pRows[i];
			_setRowElementsValue(ra, zero);
		}
	}

	//Get the memory pointer of a matrix element A(i,j)
	inline double3x3* getMatrixElement(const int i, const int j)
	{
		return _getMatrixElement(i, j);
	}

	inline const double3x3* getMatrixElement(const int i, const int j) const
	{
		CSparseMatrix33 &a = (CSparseMatrix33 &)(*this);
		return (const double3x3*)a._getMatrixElement(i, j);
	}

	//Return the diagonal elements in a 1D vector
	inline void getDiagonalMatrix(Vector3d *D) const
	{
		CSparseMatrix33& A = (CSparseMatrix33&)(*this);
		for (int i=0; i<m_nRows; i++){
			const double3x3* p = (const double3x3*)A.getMatrixElement(i, i);
			const double *x = p->x;
			D[i].x = x[0]; D[i].y=x[4]; D[i].z=x[8];
		}
	}

	// D = A:diagonal,   A = A-D
	inline void extractDiagonalMatrix(Vector3d *D)
	{
		CSparseMatrix33& A = (CSparseMatrix33&)(*this);
		for (int i=0; i<m_nRows; i++){
			double3x3* p = A.getMatrixElement(i, i);
			double *x = p->x;
			D[i].x=x[0]; D[i].y=x[4]; D[i].z=x[8]; 
			x[0] = x[4] = x[8] = 0;
		}
	}

	//Given the exiting component A(i, j) in the matrix with the input data
	inline double* addWithMatrixElement(const int i, const int j, const double& data)
	{
		double3x3* p = getMatrixElement(i, j);
		if (p){
			(*p).x[0] += data; return &(p->x[0]);
		}
		//the element is not in the matrix yet, we need to add an entry into the matrix
		CLinkedListNode *pnode = (CLinkedListNode*)Malloc(sizeof(CLinkedListNode));
		m_nElementCount++;
		pnode->m_data.setZeroMatrix();
		pnode->m_data.x[0] = data; pnode->m_nColID = j;
		_appendToMatrix(i, j, pnode);
		return &(pnode->m_data.x[0]);
	}

	//Given the exiting component A(i, j) in the matrix with the input data
	inline double3x3* addWithMatrixElement(const int i, const int j, const double3x3& data)
	{
		double3x3* p = getMatrixElement(i, j);
		if (p){
			*p += data; return p;
		}
		//the element is not in the matrix yet, we need to add an entry into the matrix
		CLinkedListNode *pnode = (CLinkedListNode*)Malloc(sizeof(CLinkedListNode));
		m_nElementCount++;
		pnode->m_data = data; pnode->m_nColID = j;
		_appendToMatrix(i, j, pnode);
		return &pnode->m_data;
	}

	//Add a diagonal matrix, the result is stored in the matrix
	inline void addDiagonalMatrix(const Vector3d *M)
	{
		if (m_nDataType == 0){ //0: double , 1: double3x3 matrix
			for (int i=0; i<m_nRows; i++){
				double3x3* p = getMatrixElement(i, i);
				p->x[0] += M[i].x;
			}
		}
		else{
			for (int i=0; i<m_nRows; i++){
				double3x3* p = getMatrixElement(i, i);
				double *x = p->x;
				const Vector3d& e = M[i];
				x[0]+=e.x; x[4]+=e.y; x[8]+=e.z;
			}
		}
	}

	inline void addDiagonalMatrix(const double *M, const double scale=1.0)
	{
		if (m_nDataType == 0){ //0: double , 1: double3x3 matrix
			for (int i=0; i<m_nRows; i++){
				double3x3* p = getMatrixElement(i, i);
				*(p->x) += M[i]*scale;
			}
		}
		else{
			for (int i=0; i<m_nRows; i++){
				double3x3* p = getMatrixElement(i, i);
				double *x = p->x;
				const double e = M[i]*scale;
				x[0]+=e; x[4]+=e; x[8]+=e;
			}
		}
	}

	//Add an identity matrix, the result is stored in the matrix
	inline void addIdentityMatrix(void)
	{
		for (int i=0; i<m_nRows; i++){
			double3x3* p = getMatrixElement(i, i);
			ASSERT0(p!=NULL);
			double *x = p->x;
			x[0]++; x[4]++; x[8]++;
		}
	}

	//Vector is on the right side of matrix:  out = A * v
	inline void multiplyVector(const Vector3d *in_vec, Vector3d *out_vec) const
	{
		//0: double, 1: matrix3x3
		if (m_nDataType==0){
			for (int j=0; j<m_nRows; j++)
				_multiplyRowWithVectorDouble(j, in_vec, out_vec[j]);
		}
		else{
			for (int j=0; j<m_nRows; j++)
				_multiplyRowWithVectorMatrix(j, in_vec, out_vec[j]);

		}
	}

	inline void multiplyVector(const Vector3d *in_vec, const int in_vec_stride_byte, Vector3d *out_vec) const
	{
		//0: double, 1: matrix3x3
		if (m_nDataType==0){
			for (int j=0; j<m_nRows; j++)
				_multiplyRowWithVectorDouble(j, in_vec, in_vec_stride_byte, out_vec[j]);
		}
		else{
			for (int j=0; j<m_nRows; j++)
				_multiplyRowWithVectorMatrix(j, in_vec, in_vec_stride_byte, out_vec[j]);
		}
	}

	//A = Diag * A
	inline void multiplyDiagonalMatrixOnLeft(const Vector3d *diagmat)
	{
		for (int i=0; i<m_nRows; ++i)
			_scaleRowWithVector(i, diagmat[i]);
	}

	//A = A * k
	inline void multplyScalar(const double& k)
	{
		const Vector3d s(k);
		for (int i=0; i<m_nRows; ++i)
			_scaleRowWithVector(i, s);
	}

	//add a matrix to itself. Attention: the matrix A must be the same 
	//structure as the original one.
    inline CSparseMatrix33& operator += (const CSparseMatrix33& A)
	{
		for (int i=0; i<m_nRows; ++i){
			CRowPointer* ra = &m_pRows[i];
			const CRowPointer* rb = &A.m_pRows[i];
			_addRowWithAnotherRow(ra, rb);
		}
		return *this;
	}

	//0: single double value
	//1: 3 X 3 matrix of double values
	inline int getMatrixElementType(void) const
	{
		return m_nDataType;
	}

	inline void setMatrixElementType(const int dtype)
	{
		m_nDataType = dtype;
	}

	inline int getElementCount(void) const
	{
		return m_nElementCount;
	}

	double getElementComponetMaxAbsValue(void);

	int getAllElementsInOneRow(const int rowid, int colID[], double val[]);

	int deleteZeroElements(void);

	//mult vector function, return to another vector
	friend inline void mult(const CSparseMatrix33 &A, const double *in_vec, double *out_vec)
	{
		A.multiplyVector((const Vector3d *)in_vec, (Vector3d *)out_vec);
	}

	//a simple diagonal preconditioner 
	friend inline void precond_mult(CSparseMatrix33& A, const double *x, const int N, double *y)
	{
		const Vector3d *p = (const Vector3d *)x;
		Vector3d *q = (Vector3d *)y;
		for (int i=0; i<A.m_nRows; i++){
			//const double s = -1.0 * A.getMatrixElement(i, i)->x[0];
			//q[i] = p[i]; // * s;
			assert(0);
		}
	}

	//Frobenius distance btw 2 matrices, they must be the same size
	friend double Distance2(const CSparseMatrix33 &A, const CSparseMatrix33 &B);
	friend inline double Distance(const CSparseMatrix33 &A, const CSparseMatrix33 &B)
	{
		double dist2 = Distance2(A, B);
		return sqrt(dist2);
	}

	friend double Distance2Diagonal(const CSparseMatrix33 &A, const CSparseMatrix33 &B);
	friend double Distance2DiagonalBlock(const CSparseMatrix33 &A, const CSparseMatrix33 &B);

	//======Symbolic computation=================================
	friend void BiQuadraticSymbolicDistance(
	    const int distType, 
		const CSparseMatrix33 &L, const CSparseMatrix33 &M, 
		const CSparseMatrix33 &K, 
		Vector6d& cofficients);
};

#endif
