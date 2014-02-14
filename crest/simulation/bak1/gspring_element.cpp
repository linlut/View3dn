//FILE: gspring_element.cpp
#include "gspring_element.h"

void CGspringElement::_computeNodalForce( 
		const Vector3d &TXI,				//node i position
		const CMeMaterialProperty &material,//material property for element
		Vector3d &fi,						//elastic force for node i
											//elastic force for node j is Fj = -Fi;
		Tensor3x3 *stiffness				//stiffness matrix
		)
{
	const double Y = material.getYoung();
	const double stress = Y / m_restLength;
	const Vector3d force = m_XI - TXI;
	fi = force*(stress*m_crossArea);
	
	//If stiffness matrix is required
	if (stiffness){	
		Tensor3x3 & m = *stiffness;
		m.IdentityMatrix();
	}
}


double CGspringElement::computeElementEnergy(const CMeMaterialProperty &material) const
{
	double engr = 0;
	//engr *= m_vol*0.5;	
	return engr;
}


void CGspringElement::exportState(const CMeMaterialProperty &material, FILE *fp) const
{
	const double energy = computeElementEnergy(material);
	fprintf(fp, "Position: %lg %lg %lg\n", m_XI.x, m_XI.y, m_XI.z);
	fprintf(fp, "Total Energy: %lg\n", energy);
}



static int test_func(void)
{
	CMeMaterialProperty mat;
	mat.setPoisson(0.30);
	//--------------------------------
	const double vol=0.0001;
	Vector3d p0(1,0,0);
	const Vector3d p1(0,0,0);
	CGspringElement e(p0, p1, vol);
	//--------------------------------
	const int N=1;
	const int K = 360.0/N;
	const double A1 = (PI*2.0)*K/360.0;
	printf("Angle\t\tForce\t\t\tNorm\tEng\n");
	for (int i=0; i<=N; i++){
		double angle = i*A1;
		double R=1;
		Vector3d Force;
		double x = R*cos(angle);
		double y = R*sin(angle);
		p0=Vector3d(x, y, 0);
		e.computeNodalForce(p0, p1, mat, Force, NULL);
		const double f2 = Magnitude(Force);
		const double engr = e.computeElementEnergy(mat);
		printf("%d, %lg, %lg, %lg, %lg, %lg\n", i*K, Force.x, Force.y, Force.z, f2, engr);
	}
	return 1;
}


static int test = test_func();

