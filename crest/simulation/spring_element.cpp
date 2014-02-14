//FILE: spring_element.cpp

#include "spring_element.h"


inline void CSpringElement::
_computeElasticForce(const Vector3d &p0, const Vector3d &p1, Vector3d &f0, Vector3d &Xij)
{
	Xij = p0 - p1;
	const double l = Magnitude(Xij);
	const double l1 = 1.0/l;
	const Vector3d n = Xij * l1;
	const double force = (m_restLength-l)*m_K;
	f0 = n* force;
}


void CSpringElement::_init(const Vector3d& p0, const Vector3d& p1)
{
	m_restLength = Distance(p0, p1);
}


void CSpringElement::computeNodalForce( 
	const Vector3d &p0, 
	const Vector3d &p1, 
	Vector3d &f0,				//elastic force for node i
	double3x3 *pstiffness)		//stiffness matrix
{
	Vector3d Xij;
	_computeElasticForce(p0, p1, f0, Xij);
	if (pstiffness){

	}
}

//if damping is required, we compute additional damping force in the 
//returned force vector
void CSpringElement::computeNodalForce( 
	const Vector3d &p0,			//node 0's world position
	const Vector3d &p1,			//node 1's world position
	const Vector3d &v0,			//node 0's velocity
	const Vector3d &v1,			//node 1's velocity
	const double Kvd,			//velocity damping ratio
	Vector3d &f0,				//elastic force for node 0
	double3x3 *pstiffness)
{
	Vector3d Xij;
	_computeElasticForce(p0, p1, f0, Xij);

	if (Kvd > 0){
		Vector3d Vij = v0 - v1;
		const double dot1 = DotProd(Vij, Xij);
		const double dot2 = DotProd(Xij, Xij);
		Vector3d Fvisco = (-Kvd*dot1/dot2)*Xij;
		f0+=Fvisco;
	}
	if (pstiffness){

	}
}


static int test_func(void)
{
	int vid[2]={0,1};
	Vector3d p0(0,0,1);
	const Vector3d p1(0,0,0);
	const double K=1;
	CSpringElement e(vid, p0, p1, K);
	const int N=20;
	double3x3 jac;
	Vector3d Force;
	double x ;
	double y ;

	for (int i=0; i<N; i++){
		x = (double)i/N;
		y = sqrt(1-x*x);
		p0=Vector3d(0, 0, y);
		//e.computeNodalForce(p0, p1, Force, &jac);
	}
	return 1;
}


static int test = test_func();

