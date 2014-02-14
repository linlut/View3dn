//FILE: truss_element.cpp

#include "truss_element.h"


inline void CTrussElement::
_computeElasticForce(const Vector3d &p0, const Vector3d &p1, Vector3d &f0, Vector3d &Xij)
{
	Xij = p0 - p1;
	double l = Magnitude(Xij);
	double l1 = 1.0/l;
	Vector3d n = Xij * l1;
	double force = (m_restLength-l)*m_K;
	f0 = n* force;
}

void CTrussElement::computeNodalForce( 
	const Vector3d &p0, 
	const Vector3d &p1, 
	Vector3d &f0,				//elastic force for node i
	double3x3 *stiffness		//stiffness matrix
	    )
{
	Vector3d Xij;
	_computeElasticForce(p0, p1, f0, Xij);
}


void CTrussElement::_init(const Vector3d& p0, const Vector3d& p1)
{
	m_restLength = Distance(p0, p1);
}


//if damping is required, we compute additional damping force in the 
//returned force vector
void CTrussElement::computeNodalForce( 
	const Vector3d &p0,			//node 0's world position
	const Vector3d &p1,			//node 1's world position
	const Vector3d &v0,			//node 0's velocity
	const Vector3d &v1,			//node 1's velocity
	Vector3d &f0,				//elastic force for node 0
	double3x3 *stiffnessi
	)
{
	Vector3d Xij;
	_computeElasticForce(p0, p1, f0, Xij);

	if (m_Kd>1e-10){
		Vector3d Vij = v0 - v1;
		double dot1 = DotProd(Vij, Xij);
		double dot2 = DotProd(Xij, Xij);
		Vector3d Fvisco = (m_Kd*dot1/dot2)*Xij;
		f0+=Fvisco;
	}
}


static int test_func(void)
{
	int vid[2]={0,1};
	Vector3d p0(0,0,1);
	const Vector3d p1(0,0,0);
	const double K=1;
	CTrussElement e(vid, p0, p1, K);
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

