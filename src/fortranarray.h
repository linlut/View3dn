#ifndef __INC_FORTRANARRAY_H__
#define __INC_FORTRANARRAY_H__

#include <sysconf.h>
#include <vector3d.h>


#define FREAL double
#define _MAXTRIX__BUFFLEN 36


template <class T> class Matrix1D
{
	
private:
	int m_nRow;
	T m_nBuffer[_MAXTRIX__BUFFLEN];
	T *m_pElement;

	void _deleteBuffer()
	{
		if (m_pElement!=m_nBuffer)
			SafeDeleteArray(m_pElement);
		m_pElement = NULL;
	}

public:
	void reSize(const int nx)
	{
		const int nsize = nx;
		if (nsize>_MAXTRIX__BUFFLEN){
			_deleteBuffer();
			m_pElement = new T[nsize];
			assert(m_pElement!=NULL);
		}
		else
			m_pElement = m_nBuffer;

		m_nRow = nx; 
		for (int i=0; i<nsize; i++)
			m_pElement[i]=0;
	}

	Matrix1D(void)
	{
		m_pElement = m_nBuffer;
		m_nRow = _MAXTRIX__BUFFLEN;
		for (int i=0; i<_MAXTRIX__BUFFLEN; i++) 
			m_pElement[i]=0;
	}

	Matrix1D(const int nx)
	{
		m_pElement = NULL;
		reSize(nx);
	}

	~Matrix1D()
	{
		_deleteBuffer();
	}

	T& operator()(const int x)
	{
		const int x1= x-1;
		ASSERT0(x1>=0 && x1<m_nRow);
		return m_pElement[x1];
	}

	void setValue(const T& val)
	{
		for (int i=0; i<m_nRow; i++)
			m_pElement[i]=val;
	}
};


template <class T> class Matrix2D
{
	
private:
	int m_nRow;
	int m_nCol;
	T m_nBuffer[_MAXTRIX__BUFFLEN];
	T *m_pElement;
	int m_nBorrowedSpace;

	void _deleteBuffer()
	{
		if (m_pElement!=m_nBuffer)
			if (!m_nBorrowedSpace)
				SafeDeleteArray(m_pElement);
		m_pElement = NULL;
		m_nBorrowedSpace = false;
	}

public:
	void reSize(const int nx, const int ny)
	{
		if (m_nBorrowedSpace) //if memory pointer is borrowed, resize is not permitted
			assert(0);

		const int nsize = (nx)*(ny);
		if (nsize>_MAXTRIX__BUFFLEN){
			_deleteBuffer();
			m_pElement = new T[nsize];
			assert(m_pElement!=NULL);
		}
		else
			m_pElement = m_nBuffer;

		m_nRow = nx; m_nCol = ny;
		for (int i=0; i<nsize; i++)
			m_pElement[i]=0;
	}

	Matrix2D(void)
	{
		m_nBorrowedSpace = false;
		m_pElement = m_nBuffer;
		m_nRow = 4;
		m_nCol = 4;
		for (int i=0; i<_MAXTRIX__BUFFLEN; i++) 
			m_pElement[i]=0;
	}

	Matrix2D(const int nx, const int ny)
	{		
		m_nBorrowedSpace = false;
		m_pElement = NULL;
		reSize(nx, ny);
	}

	~Matrix2D()
	{
		_deleteBuffer();
	}

	int indexOf(const int x, const int y)
	{
		const int x1= x-1;
		const int y1= y-1;
		ASSERT0(x1>=0 && x1<m_nRow);
		ASSERT0(y1>=0 && y1<m_nCol);
		const int index = x1*m_nCol+y1;
		return index;
	}

	T& operator()(const int x, const int y)
	{
		const int index = indexOf(x,y);
		return m_pElement[index];
	}

	void setValue(const T& val)
	{
		const int nsize = m_nRow*m_nCol;
		for (int i=0; i<nsize; i++)
			m_pElement[i]=val;
	}

};


template <class T> 
class Matrix3D
{
private:
	int m_nRow;
	int m_nCol;
	int m_nDepth;
	T m_nBuffer[_MAXTRIX__BUFFLEN];
	T *m_pElement;
	int m_nBorrowedSpace;

	void _deleteBuffer()
	{
		if (m_pElement!=m_nBuffer)
			if (!m_nBorrowedSpace)
				SafeDeleteArray(m_pElement);
		m_pElement = NULL;
		m_nBorrowedSpace = false;
	}

public:
	void reSize(const int nx, const int ny, const int nz)
	{
		assert(!m_nBorrowedSpace);
		const int nsize = nx*ny*nz;
		if (nsize>_MAXTRIX__BUFFLEN){
			_deleteBuffer();
			m_pElement = new T[nsize];
			assert(m_pElement!=NULL);
		}
		else
			m_pElement = m_nBuffer;

		m_nRow = nx; m_nCol = ny; m_nDepth=nz;
		for (int i=0; i<nsize; i++)
			m_pElement[i]=T(0);
	}

	Matrix3D(void)
	{
		m_pElement = m_nBuffer;
		m_nRow = 2;
		m_nCol = 2;
		m_nDepth = 2;
		m_nBorrowedSpace = false;
		for (int i=0; i<_MAXTRIX__BUFFLEN; i++) 
			m_pElement[i]=T(0);
	}

	Matrix3D(const int nx, const int ny, const int nz)
	{
		m_pElement = NULL;
		m_nBorrowedSpace = false;
		reSize(nx, ny, nz);
	}

	Matrix3D(const int nx, const int ny, const int nz, T *p)
	{
		m_pElement = p;
		m_nBorrowedSpace = true;
		m_nRow = nx;
		m_nCol = ny;
		m_nDepth = nz;		
	}

	~Matrix3D()
	{
		_deleteBuffer();
	}

	int indexOf(const int x, const int y, const int z)
	{
		const int x1= x-1;
		const int y1= y-1;
		const int z1= z-1;
		ASSERT0(x1>=0 && x1<m_nRow);
		ASSERT0(y1>=0 && y1<m_nCol);
		ASSERT0(z1>=0 && z1<m_nDepth);
		const int index = (x1*m_nCol+y1)*m_nDepth+z1;
		return index;
	}

	T& operator()(const int x, const int y, const int z)
	{
		const int index = indexOf(x,y,z);
		return m_pElement[index];
	}

	Matrix3D<T> operator()(const int x)
	{
		const int x1= x-1;
		ASSERT0(x1>=0 && x1<m_nRow);
		const int index = x1*m_nCol*m_nDepth;
		T *p = &m_pElement[index];
		Matrix3D<T> m(m_nRow-x1, m_nCol, m_nDepth, p);
		return m;
	}

	void setValue(const T& val)
	{
		const int nsize = m_nRow*m_nCol*m_nDepth;
		for (int i=0; i<nsize; i++)
			m_pElement[i]=val;
	}

	int getMatrixSize(void)
	{
		const int nsize = m_nRow*m_nCol*m_nDepth;
		return nsize;
	}

	void getMatrixDimension(int &nx, int &ny, int& nz)
	{
		nx = m_nRow;
		ny = m_nCol;
		nz = m_nDepth;
	}
};


#define M1 Matrix1D<FREAL>
#define IA1 Matrix1D<int>
 
#define M2 Matrix2D<FREAL> 
#define M3 Matrix3D<FREAL> 

#define MV3 Matrix3D<Vector3d> 



//ANGLIMIT is 180 or 360 degree
inline void 
GET_ANGULAR_SECTIONS(const FREAL& a1, const FREAL& a2, const int ANGLIMIT, int &nzone, FREAL& zoneangle)
{
	FREAL angdif = a2 - a1;
	if (angdif>ANGLIMIT) angdif=ANGLIMIT;
	else if (angdif<0) angdif=0;

	const FREAL fcc = angdif/90.0;
	const int ncc = (int)fcc;
	if (fabs(fcc-ncc)<1e-8){
		nzone = ncc;
		zoneangle=90;
	}
	else{
		nzone = ncc+1;
		zoneangle = angdif/nzone;
	}
}

inline void
FAST_ROTATE_Z_AXIS(Vector3d v[], const int nv, const FREAL &alpha)
{
	int i;
	if (alpha==90){
		for (i=0; i<nv; i++){
			Vector3d *p=&v[i];
			double _tmp = p->x;
			p->x = -p->y;
			p->y = _tmp;
		}
	}
	else{
		const double A = PI*(alpha/180.0);
		const FREAL cosa = cos(A);
		const FREAL sina = sin(A);
		for (i=0; i<nv; i++){
			Vector3d *p=&v[i];
			const FREAL x = p->x;
			const FREAL y = p->y;
			p->x = cosa*x-sina*y;
			p->y = sina*x+cosa*y;
		}
	}
}


//=============Generate a layer for the 1/4 cylinder=================

inline void 
Vector3dToFMatrix(const Vector3d& vv, M3 & m, const int i, const int zone)
{
	m(1,i,zone) = vv.x; m(2,i,zone) = vv.y; m(3,i,zone) = vv.z;
}


extern void 
meshSlantBrickVertices(const Vector3d cubevert[8], const int nx, const int ny, const int nz, Vector3d * &meshvert, int &nv);

extern void 
sprmshzn(const int nsplit, M1& alpha, M1& beta, const FREAL& r, MV3& vertex);

extern void
meshSphereVertices(const Vector3d& ct, const FREAL r, 
			const FREAL& a0, const FREAL& a1, 
			const FREAL& b0, const FREAL& b1, 
			const int nx, const int ny, 
			Vector3d * &meshvert, int &nv);


//=============Generate a layer for the 1/4 cylinder=================
extern void 
colmshzn(const int nsplit, const FREAL &r, M1& alpha, const FREAL &h, MV3 &vertex);

extern void 
meshCylinderVertices(const Vector3d& ct, const FREAL r, 
			const FREAL& a0, const FREAL& a1, 
			const FREAL& h, 
			const int nsplitw, const int nsplith, 
			Vector3d * &meshvert, int &nv);



//=============Mesh 1/4 sphere shell
extern void 
shereShell(const int nsplitw, const int nsplith, const FREAL& r, const FREAL& h,
	const FREAL &a1, const FREAL &a2, const FREAL &b1, const FREAL& b2, 
	MV3 &vertex);


extern void
cylinderShell(const int nsplitw, int nsplitt, const int nsplith, 
	const FREAL &r, const FREAL &h, const FREAL &th,
	const FREAL &a1, const FREAL &a2, 
	MV3 &vertex);


extern void 
tetrahedronMesher(const int nsplitx, const int nsplity, const int nsplitz, const Vector3d v[4], MV3 &vertex);


extern void 
coneMesher(const int nsplitw, const int nsplith, M1& alpha, const FREAL& r, const FREAL &h, MV3& vertex);

//mesh a 6-vertex prism
extern void 
prismMesher(M2 & xyzo, const FREAL h, const int nArcDiv, MV3& vertex);
extern void 
prismMesher(const Vector3d v[6], const int nArcDiv, MV3& vertex);


//interface for 20 vertex mesher
extern void 
msh20zn(const Vector3d v[20], const int nsx, const int nsy, const int nsz, MV3& vertex);




#endif

