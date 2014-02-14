//FILE: thinsehll_element.cpp
#include <geomath.h>
#include <ztime.h>
#include "thinshell3_element.h"
#include "simu_entity.h"


template <class T> 
class Matrix2D
{
private:
	int m_nRow;
	int m_nCol;
	T *m_pElement;

public:
	Matrix2D(const int nrow, const int ncol, T* buffer)
	{		
		m_nRow = nrow; m_nCol = ncol;
		m_pElement = buffer;
	}

	inline int indexOf(const int x, const int y) const
	{
		return (x*m_nCol + y);
	}

	//C style addressing, start from 0
	inline T& operator ()(const int x, const int y)
	{
		return m_pElement[indexOf(x,y)];
	}

	inline T& operator ()(const int x, const int y) const
	{
		return m_pElement[indexOf(x,y)];
	}
	
	inline friend void Transpose(const Matrix2D &a, Matrix2D& b)
	{
		b.m_nRow = a.m_nCol; 
		b.m_nCol = a.m_nRow;
		for (int i=0; i<a.m_nRow; i++)
			for (int j=0; j<a.m_nCol; j++)
				b(j, i) = a(i,j);
	}

	inline friend void Mult(const Matrix2D& a, const Matrix2D& b, Matrix2D& c)
	{
		ASSERT0(a.m_nCol == b.m_nRow);
		c.m_nRow = a.m_nRow;
		c.m_nCol = b.m_nCol;
		for (int i=0; i<c.m_nRow; i++){
			for (int j=0; j<c.m_nCol; j++){
				T sum = a(i,0)*b(0,j);
				for (int k=1; k<a.m_nCol; k++) sum+=a(i,k)*b(k,j);
				c(i, j)=sum;
			}
		}
	}
};

void computeQuadDeriv(const Vector3d& p1, const Vector3d& p2, const Vector3d& p3, 
					  const Vector3d& p4, const Vector3d& p5, Vector3d deriv[5])
{
	Vector3d buf0[5]={p1, p2, p3, p4, p5};
	Vector3d buf1[5];
	double3x3 buf2;
	Matrix2D<double> A(5, 3, &buf0[0].x);
	Matrix2D<double> B(3, 5, &buf1[0].x);
	Matrix2D<double> C(3, 3, &buf2.x[0]);
	Matrix2D<double> D(5, 3, &deriv[0].x);
	Transpose(A, B);
	Mult(B, A, C);
	buf2.Invert();
	Mult(A, C, D);
}

void computeQuadDeriv2(
	const Vector3d& p1, const Vector3d& p2, const Vector3d& p3, 
	const Vector3d& p4, const Vector3d& p5, Vector3d deriv[5])
{
	double3x3 qq, m;
	vectorTensorProduct(p1, p1, qq);
	vectorTensorProduct(p2, p2, m); qq+=m;
	vectorTensorProduct(p3, p3, m); qq+=m;
	vectorTensorProduct(p4, p4, m); qq+=m;
	vectorTensorProduct(p5, p5, m); qq+=m;
	qq.Invert();
	qq.Transpose();
	deriv[0] = qq*p1;
	deriv[1] = qq*p2;
	deriv[2] = qq*p3;
	deriv[3] = qq*p4;
	deriv[4] = qq*p5;
}

void computeStiffnessMat(const Vector3d deriv[5], const double& K, double3x3 *ppJacobian[6])
{
	const double aa = DotProd(deriv[0], deriv[0]);
	const double ab = DotProd(deriv[0], deriv[1]);
	const double ac = DotProd(deriv[0], deriv[2]);
	const double ad = DotProd(deriv[0], deriv[3]);
	const double ae = DotProd(deriv[0], deriv[4]);
	const double bb = DotProd(deriv[1], deriv[1]);
	const double bc = DotProd(deriv[1], deriv[2]);
	const double bd = DotProd(deriv[1], deriv[3]);
	const double be = DotProd(deriv[1], deriv[4]);
	const double cc = DotProd(deriv[2], deriv[2]);
	const double cd = DotProd(deriv[2], deriv[3]);
	const double ce = DotProd(deriv[2], deriv[4]);
	const double dd = DotProd(deriv[3], deriv[3]);
	const double de = DotProd(deriv[3], deriv[4]);
	const double ee = DotProd(deriv[4], deriv[4]);
	const double S11 = aa*K;
	const double S12 = (ab+ad)*K;
	const double S13 = (ac+ae)*K;
	const double S22 = (bb+bd+bd+dd)*K;
	const double S23 = (bc+be+cd+de)*K;
	const double S33 = (cc+ce+ce+ee)*K;

	for (int i=0; i<6; i++) ppJacobian[i]->ZeroMatrix();
	double *x;
	x=ppJacobian[0]->x; x[0]=x[4]=x[8]=S11;
	x=ppJacobian[1]->x; x[0]=x[4]=x[8]=S12;
	x=ppJacobian[2]->x; x[0]=x[4]=x[8]=S13;
	x=ppJacobian[3]->x; x[0]=x[4]=x[8]=S22;
	x=ppJacobian[4]->x; x[0]=x[4]=x[8]=S23;
	x=ppJacobian[5]->x; x[0]=x[4]=x[8]=S33;
}

inline void computeStiffnessMat(const double3x3& strainDeriv, const double& K, double3x3 *ppJacobian[3]) 
{
	const Vector3d *a = (const Vector3d *)&strainDeriv.x[0];
	const Vector3d *b = (const Vector3d *)&strainDeriv.x[3];
	const Vector3d *c = (const Vector3d *)&strainDeriv.x[6];
	const double aa = DotProd(*a, *a);
	const double ab = DotProd(*a, *b);
	const double ac = DotProd(*a, *c);
	const double bb = DotProd(*b, *b);
	const double bc = DotProd(*b, *c);
	const double cc = DotProd(*c, *c);
	const double S11 = aa*K;
	const double S12 = (ab+ac)*K;
	const double S22 = (bb+cc+bc+bc)*K;

	ppJacobian[0]->ZeroMatrix();
	ppJacobian[1]->ZeroMatrix();
	ppJacobian[2]->ZeroMatrix();
	double *x;
	x=ppJacobian[0]->x; x[0]=x[4]=x[8]=S11;
	x=ppJacobian[1]->x; x[0]=x[4]=x[8]=S12;
	x=ppJacobian[2]->x; x[0]=x[4]=x[8]=S22;
}


//construct the two local coord sys. of the shell element using current nodal positions
//the aixs of the coord sys are stored in two 3x3 matrices
//static inline 
inline void getReferencePlanesForTrianglePair(
	const Vector3d &q1, const Vector3d &q2, const Vector3d &q3, 
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& mat, double &xlen)
{
	Vector3d Y0, Y1;
	Vector3d &Z0 = facenorm0;
	Vector3d &Z1 = facenorm1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	X  = q1; 
	xlen = Magnitude(X);
	X /= xlen;	//normalize
	Y0 = q2;
	Z0 = CrossProd(X, Y0); 
	Y1 = q3;	
	Z1 = CrossProd(Y1, X); 
	Z0.Normalize();
	Z1.Normalize();
	Z = Z0+Z1; Z.Normalize();
	Y = CrossProd(Z, X); 
}

//use the center plane of the dihedral angle as the reference plane
inline void getReferencePlanesForQuadPair(
	const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	Vector3d &facenorm0, Vector3d &facenorm1, double3x3& mat, double &xlen)
{
	Vector3d Y0, Y1;
	Vector3d &Z0 = facenorm0;
	Vector3d &Z1 = facenorm1;
	Vector3d &X = *((Vector3d*)(&mat.x[0]));
	Vector3d &Y = *((Vector3d*)(&mat.x[3]));
	Vector3d &Z = *((Vector3d*)(&mat.x[6]));
	const Vector3d p23 = (p2+p3)*0.5;	//quad center
	const Vector3d p45 = (p4+p5)*0.5;	//quad center
	X = p1; 
	xlen = Magnitude(X);
	X /= xlen;	//normalize
	Y0 = p23;
	Z0 = CrossProd(X, Y0); 
	Y1 = p45;	
	Z1 = CrossProd(Y1, X); 
	Z0.Normalize();
	Z1.Normalize();
	Z = Z0+Z1; Z.Normalize();
	Y = CrossProd(Z, X); 
}


const double HEIGHT_DELTA = (8.0/4096);

void CThinshellElementFEM::_initElements(
	const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const double& thickness)
{
	//compute volume for each triangle, each truss edge
	m_area0 = fabs(triangle_area(p0, p1, p2));
	m_area1 = fabs(triangle_area(p0, p1, p3));
	m_thickness = thickness;

	//construct the world/local transform matrix for the two triangles
	double3x3 rot;
	Vector3d N0, N1;
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot, m_len0);
	const Vector3d qq1(m_len0, 0, 0);
	const Vector3d qq2 = rot*q2;
	const Vector3d qq3 = rot*q3;
	m_qq2 = qq2; m_qq3 = qq3;
	const double DELTA = m_len0*HEIGHT_DELTA;
	const Vector3d qq4 = rot*(q2+DELTA*N0);
	const Vector3d qq5 = rot*(q3+DELTA*N1);

	//derivitive pF/pX
	copyVectorsToColumns(qq1, qq2, qq4, m_XInv0);
	copyVectorsToColumns(qq1, qq3, qq5, m_XInv1);
	m_XInv0.Invert();
	m_XInv1.Invert();

	//stiffness related comp.
	double3x3 jac[6];
	double3x3 *ppJacobian[6]={jac, jac+1, jac+2, jac+3, jac+4, jac+5}; 
	const double E = mtl.getYoung()*m_thickness;
	computeStiffnessMat(m_XInv0, -E*m_area0, ppJacobian);
	computeStiffnessMat(m_XInv1, -E*m_area1, ppJacobian+3);
	m_A11 = jac[0].x[0]; m_A12 = jac[1].x[0];
	m_A22 = jac[2].x[0];
	m_B11 = jac[3].x[0]; m_B12 = jac[4].x[0];
	m_B22 = jac[5].x[0];

	m_strain0.ZeroMatrix();
	m_strain1.ZeroMatrix();
}


inline double TET_VOLUME(const Vector3d &a, const Vector3d &b, const Vector3d &c)
{
	return DotProd(a, CrossProd(b, c));
}

void CThinshellElementFEM::_initElements(
	const CMeMaterialProperty &mtl, 
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	const double& thickness)
{
	//compute volume for each triangle, each truss edge
	m_area0 = fabs(quad_area(p0, p1, p2, p3));
	m_area1 = fabs(quad_area(p0, p4, p5, p1));  //right hand rule
	m_thickness = thickness;

	//construct the world/local transform matrix for the two triangles
	double3x3 rot;
	Vector3d N0, N1;
	const Vector3d p00=(p0+p1)*0.5;
	const Vector3d q1=p1-p00, q2=p2-p00, q3=p3-p00, q4=p4-p00, q5=p5-p00;
	//const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0, q4=p4-p0, q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, m_len0);
	const Vector3d qq1(m_len0, 0, 0);
	const Vector3d qq2 = rot*q2;
	const Vector3d qq3 = rot*q3;
	const Vector3d qq4 = rot*q4;
	const Vector3d qq5 = rot*q5;
	m_qq2 = qq2; m_qq3 = qq3;
	m_qq4 = qq4; m_qq5 = qq5;
	const double DELTA = m_len0*HEIGHT_DELTA;
	const Vector3d qq2u = rot*(q2+DELTA*N0);
	const Vector3d qq3u = rot*(q3+DELTA*N0);
	const Vector3d qq4u = rot*(q4+DELTA*N1);
	const Vector3d qq5u = rot*(q5+DELTA*N1);

	//derivitive pF/pX
	Vector3d *derivA = (Vector3d *)(&m_XInv0.x[0]);
	Vector3d *derivB = (Vector3d *)(&m_XInv1.x[0]);
	computeQuadDeriv2(qq1, qq2, qq3, qq2u, qq3u, derivA);
	computeQuadDeriv2(qq1, qq4, qq5, qq4u, qq5u, derivB);

	//stiffness
	double3x3 jac[12];
	double3x3 *ppJacobian[12]={jac, jac+1, jac+2, jac+3, jac+4, jac+5, jac+6, jac+7, jac+8, jac+9, jac+10, jac+11};
	const double E = mtl.getYoung()*m_thickness;
	computeStiffnessMat(derivA, -E*m_area0, ppJacobian);
	computeStiffnessMat(derivB, -E*m_area1, ppJacobian+6);
	m_A11 = jac[0].x[0]; m_A12 = jac[1].x[0]; m_A13 = jac[2].x[0];
	m_A22 = jac[3].x[0]; m_A23 = jac[4].x[0];
	m_A33 = jac[5].x[0];
	m_B11 = jac[6].x[0]; m_B12 = jac[7].x[0]; m_B13 = jac[8].x[0];
	m_B22 = jac[9].x[0]; m_B23 = jac[10].x[0];
	m_B33 = jac[11].x[0];

	m_strain0.ZeroMatrix();
	m_strain1.ZeroMatrix();
}


void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[4],					//force in global coord.
	double3x3 *ppJacobian[7])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double3x3 rot, matx0, matx1;
	Vector3d N0, N1;
	double xlen;
	const Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0;
	getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot, xlen);
	const Vector3d u1(xlen-m_len0, 0, 0);
	const Vector3d u2 = rot*q2 - m_qq2;
	const Vector3d u3 = rot*q3 - m_qq3;
	//force comp.
	FF[1] = (m_A11+m_B11)*u1 + m_A12*u2 + m_B12*u3;
	FF[2] = m_A12*u1 + m_A22*u2; 
	FF[3] = m_B12*u1 + m_B22*u3; 
	//spring force
	const double E = mtl.getYoung()*m_thickness;
	const double K= -10.0*E*(m_area0+m_area1)/(m_len0*m_len0);
	const double springf = K*u1.x;
	FF[1].x+=springf;
	//get world force
	FF[1] = FF[1]*rot;
	FF[2] = FF[2]*rot;
	FF[3] = FF[3]*rot;
	FF[0] = -(FF[1]+FF[2]+FF[3]);
	//=================================================
	if (ppJacobian[0]){
		computeStiffnessMat(m_XInv0, -E*m_area0, ppJacobian); 
		computeStiffnessMat(m_XInv1, -E*m_area1, ppJacobian+3); 
		//add spring stiffness
		ppJacobian[6]->ZeroMatrix();
		double *x = ppJacobian[6]->x; 
		x[0]= x[4] = x[8] = K;
	}
}

/*
inline void computeBendingForceTrimesh(
	const double3x3 &strain, const double3x3 &strainDeriv, const double& K, Vector3d F[2])
{
	const Vector3d* derivRow0 = (const Vector3d*)(&strainDeriv.x[0]);
	const Vector3d* derivRow1 = (const Vector3d*)(&strainDeriv.x[3]);
	const Vector3d* derivRow2 = (const Vector3d*)(&strainDeriv.x[6]);
	F[0] = strain * (*derivRow0);
	F[1] = strain * ((*derivRow1)+(*derivRow2));
	F[0]*=K;
	F[1]*=K;
}

void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, const Vector3d &p3, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[4],					//force in global coord.
	double3x3 *ppJacobian[6])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double3x3 rot, matx0, matx1;
	Vector3d N0, N1;
	double xlen;
	const Vector3d q1=p1-p0;
	const Vector3d q2=p2-p0;
	const Vector3d q3=p3-p0;
	getReferencePlanesForTrianglePair(q1, q2, q3, N0, N1, rot, xlen);
	const Vector3d qq1(xlen, 0, 0);
	const Vector3d qq2 = rot*q2;
	const Vector3d qq3 = rot*q3;
	const double DELTA = m_len0*HEIGHT_DELTA;
	const Vector3d qq4 = rot*(q2 + DELTA*N0);
	const Vector3d qq5 = rot*(q3 + DELTA*N1);
	copyVectorsToColumns(qq1, qq2, qq4, matx0);
	copyVectorsToColumns(qq1, qq3, qq5, matx1);
	double3x3 strain0 = matx0*m_XInv0; strain0.MinusIdentity();
	double3x3 strain1 = matx1*m_XInv1; strain1.MinusIdentity();

	//force comp
	Vector3d F0[2], F1[2];
	const double e = m_thickness*mtl.getYoung();
	const double k0 = -e*m_area0;
	const double k1 = -e*m_area1;
	const double youngDamp = mtl.getEffectiveYoungDamp();
	const double3x3 straindamp0 = youngDamp * (strain0 - m_strain0);
	const double3x3 straindamp1 = youngDamp * (strain1 - m_strain1);
	const double3x3 strainSum0 = strain0 + straindamp0;
	const double3x3 strainSum1 = strain1 + straindamp1;
	computeBendingForceTrimesh(strainSum0, m_XInv0, k0, F0);
	computeBendingForceTrimesh(strainSum1, m_XInv1, k1, F1);

	//save strain data
	m_strain0 = strain0;
	m_strain1 = strain1;

	//accumulate forces on the nodes
	FF[1] = F0[0]+F1[0];
	FF[2] = F0[1];
	FF[3] = F1[1];

	//spring force
	const double delta = xlen-m_len0;
	const double K= -mtl.getYoung()*m_thickness*(m_area0+m_area1)/m_len0*1500;
	const double springf = K*delta;
	FF[1].x+=springf;

	//get world force
	FF[1] = FF[1]*rot;
	FF[2] = FF[2]*rot;
	FF[3] = FF[3]*rot;
	FF[0] = -(FF[1]+FF[2]+FF[3]);

	if (ppJacobian[0]){
		computeStiffnessMat(m_XInv0, k0, ppJacobian); 
		computeStiffnessMat(m_XInv1, k1, ppJacobian+3); 
		//add spring stiffness
		double *x = ppJacobian[6]->x; 
		x[0]= x[4] = x[8] = K;
	}
}
*/


inline void isReflected2(const double det0, const double det, const Vector3d &n, Vector3d &p0, Vector3d &p1)
{
	if (det0==0){
		if (det<0) goto DO_ADJUST;
	}
	else if (det0*det<0){
		goto DO_ADJUST;
	}
	goto RETURNP;

DO_ADJUST:
	double dist = DotProd(p0, n)*2.0;
	p0 -= dist*n;
	dist = DotProd(p1, n)*2.0;
	p1 -= dist*n;

RETURNP:
	return;
}

void CThinshellElementFEM::computeForce(
	const Vector3d &p0, const Vector3d &p1, const Vector3d &p2, 
	const Vector3d &p3, const Vector3d &p4, const Vector3d &p5, 
	const int isStatic,				//static simulation or dynamic simu
	const CMeMaterialProperty &mtl, //material parameters
	Vector3d FF[6],					//force in global coord.
	double3x3 *ppJacobian[13])		//jocobian matrices, if needed (not NULL)
{
	//gen. the world/local and local/world transformation matrices
	double3x3 rot, matx0, matx1;
	Vector3d N0, N1;
	double xlen;
	const Vector3d p00=(p0+p1)*0.5;
	Vector3d q1=p1-p00, q2=p2-p00, q3=p3-p00, q4=p4-p00, q5=p5-p00;
	//Vector3d q1=p1-p0, q2=p2-p0, q3=p3-p0, q4=p4-p0, q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, xlen);
	const Vector3d u1(xlen-m_len0, 0, 0);
	const Vector3d u2 = rot*q2 - m_qq2;
	const Vector3d u3 = rot*q3 - m_qq3;
	const Vector3d u4 = rot*q4 - m_qq4;
	const Vector3d u5 = rot*q5 - m_qq5;
	//force comp.
	FF[1] = (m_A11+m_B11)*u1 + m_A12*u2 + m_A13*u3 + m_B12*u4 + m_B13*u5;
	FF[2] = m_A12*u1 + m_A22*u2 + m_A23*u3; 
	FF[3] = m_A13*u1 + m_A23*u2 + m_A33*u3; 
	FF[4] = m_B12*u1 + m_B22*u4 + m_B23*u5; 
	FF[5] = m_B13*u1 + m_B23*u4 + m_B33*u5; 
	//spring force
	const double E = mtl.getYoung()*m_thickness;
	const double K= -0.0*E*(m_area0+m_area1)/(m_len0*m_len0);
	const double springf = K*u1.x;
	FF[1].x+=springf;
	//get world force
	FF[1] = FF[1]*rot;
	FF[2] = FF[2]*rot;
	FF[3] = FF[3]*rot;
	FF[4] = FF[4]*rot;
	FF[5] = FF[5]*rot;
	FF[0] = -(FF[1]+FF[2]+FF[3]+FF[4]+FF[5]);
	//=================================================
	if (ppJacobian[0]){
		const Vector3d* derivA = (const Vector3d*)m_XInv0.x;
		const Vector3d* derivB = (const Vector3d*)m_XInv1.x;
		computeStiffnessMat(derivA, -E*m_area0, ppJacobian);
		computeStiffnessMat(derivB, -E*m_area1, ppJacobian+6);
		//add spring stiffness
		ppJacobian[12]->ZeroMatrix();
		double *x = ppJacobian[12]->x; 
		x[0]= x[4] = x[8] = K;
	}
}


	/*

inline void computeBendingForceQuadmesh(const double3x3 &strain, const double3x3 &strainDeriv, const double& K, Vector3d F[3])
{
	const Vector3d* derivRow0 = (const Vector3d*)(&strainDeriv.x[0]);
	const Vector3d* derivRow1 = (const Vector3d*)(&strainDeriv.x[3]);
	const Vector3d* derivRow2 = (const Vector3d*)(&strainDeriv.x[6]);
	F[0] = strain * (*derivRow0);
	F[1] = strain * ((*derivRow1));
	F[2] = strain * ((*derivRow2));
	F[0]*=K;
	F[1]*=K;
	F[2]*=K;
}


	//construct the world/local transform matrix for the two triangles
	double3x3 rot, matx0, matx1, strain0, strain1;
	Vector3d N0, N1;
	double xlen;
	const Vector3d q1=p1-p0;
	Vector3d q2=p2-p0;
	Vector3d q3=p3-p0;
	Vector3d q4=p4-p0;
	Vector3d q5=p5-p0;
	getReferencePlanesForQuadPair(q1, q2, q3, q4, q5, N0, N1, rot, xlen);
	const Vector3d qq1(xlen, 0, 0);
	const double DELTA = m_len0*HEIGHT_DELTA;
	Vector3d qq2 = rot*q2;
	Vector3d qq3 = rot*(q3+DELTA*N0);
	Vector3d qq4 = rot*q4;
	Vector3d qq5 = rot*(q5+m_delta1*N1);
	copyVectorsToColumns(qq1, qq2, qq3, matx0);
	copyVectorsToColumns(qq1, qq4, qq5, matx1);

	//need to check reflection
	const Vector3d *ZZ = (const Vector3d *)(&rot.x[6]);
	if (isReflected(m_signXInv0, matx0, N0, q2, q3)){
		qq2 = rot*q2;
		qq3 = rot*q3;
		copyVectorsToColumns(qq1, qq2, qq3, matx0);
	}
	if (isReflected(m_signXInv1, matx1, N1, q4, q5)){
		qq4 = rot*q4;
		qq5 = rot*q5;
		copyVectorsToColumns(qq1, qq4, qq5, matx1);
	}
	strain0 = matx0*m_XInv0; strain0.MinusIdentity();
	strain1 = matx1*m_XInv1; strain1.MinusIdentity();

	//force comp
	Vector3d F0[3], F1[3];
	const double e = m_thickness*mtl.getYoung();
	const double k0 = -e*m_area0;
	const double k1 = -e*m_area1;
	const double youngDamp = mtl.getEffectiveYoungDamp();
	const double3x3 straindamp0 = youngDamp * (strain0 - m_strain0);
	const double3x3 straindamp1 = youngDamp * (strain1 - m_strain1);
	const double3x3 strainSum0 = strain0 + straindamp0;
	const double3x3 strainSum1 = strain1 + straindamp1;
	computeBendingForceQuadmesh(strainSum0, m_XInv0, k0, F0);
	computeBendingForceQuadmesh(strainSum1, m_XInv1, k1, F1);

	//save strain data
	m_strain0 = strain0;
	m_strain1 = strain1;

	//accumulate forces on the nodes
	FF[1] = F0[0]+F1[0];
	FF[2] = F0[1];
	FF[3] = F0[2];
	FF[4] = F1[1];
	FF[5] = F1[2];
	FF[1] = FF[1]*rot;
	FF[2] = FF[2]*rot;
	FF[3] = FF[3]*rot;
	FF[4] = FF[4]*rot;
	FF[5] = FF[5]*rot;
	FF[0] = -(FF[1]+FF[2]+FF[3]+FF[4]+FF[5]);

	if (ppJacobian[0]){
	}
	*/

bool CThinshellElementFEM::hasConstantStiffness(void)
{ 
	return false; 
}


static int test2()
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1);
	const int isstatic=0;
	const int pNodeIDBuffer[6]={0, 1, 2, 3, 4, 5};
	const double x=1;
	Vector3d p[6]={
		Vector3d(0, 0, 0),
		Vector3d(x, 0, 0),
		Vector3d(x, 1, 0),
		Vector3d(0, 1, 0),
		Vector3d(0, -1, 0),
		Vector3d(x, -1, 0)
	};

	const double thickness=1;

	CThinshellElementFEM e(mtl, isstatic, pNodeIDBuffer, p[0], p[1], p[2], p[3], p[4], p[5], thickness);

	Vector3d F[6];
	double3x3 *ppJacobian[13]={NULL};

	const double r = sqrt(2.0)/2;
	const double d = 0e-2;
	p[2]=Vector3d(x, r, r-d);
	p[3]=Vector3d(0, r, r+d);
	p[4]=Vector3d(0, -r, r+d);
	p[5]=Vector3d(x, -r, r-d);
	e.computeForce(p[0], p[1], p[2], p[3], p[4], p[5], 0, mtl, F, ppJacobian);

	return 1;
}


static int test1()
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1);
	const int isstatic=0;
	const int pNodeIDBuffer[6]={0, 1, 2, 3, 4, 5};
	const double x=1;
	Vector3d p[4]={
		Vector3d(0, 0, 0),
		Vector3d(x, 0, 0),
		Vector3d(x/2, 1, 0),
		Vector3d(x/2, -1, 0)
	};
	const double thickness=1;
	CThinshellElementFEM e(mtl, isstatic, pNodeIDBuffer, p[0], p[1], p[2], p[3], thickness);

	Vector3d F[4];
	double3x3 *ppJacobian[13]={NULL};

	const double r = sqrt(2.0)/2;
	const double d = 0e-2;
	p[2]=Vector3d(x/2, r, (r-d));
	p[3]=Vector3d(x/2, -r, (r+d));
	e.computeForce(p[0], p[1], p[2], p[3], 0, mtl, F, ppJacobian);

	return 1;
}

static int r=test2();
