//FILE: gspring_element.h

#ifndef _INC_GSPRING_ELM_H_
#define _INC_GSPRING_ELM_H_

#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "shear_element.h"


class CGspringElement
{
private:
	Vector3d m_XI;			//Node 0 position in initial configuration	
	double m_restLength;	//volume of the truss
	double m_crossArea;		//cross section area

private:
	void _init(const Vector3d& p0, const Vector3d& p1, const double3x3* pmat, const double &vol)
	{
		if (pmat==NULL)
			m_XI = p0 - p1;
		else
			m_XI = (*pmat)*(p0-p1);

		m_restLength = Distance(p0, p1);
		m_crossArea = vol/m_restLength;
	}

	//elastic force for node i
	//elastic force for node j is Fj = -Fi;
	//stiffness matrix for node i, if the pointer is not NULL
	void _computeNodalForce(const Vector3d &TXI, const CMeMaterialProperty &matril, 
		Vector3d &fi, Tensor3x3 *stiffnessi=NULL);

public:
	CGspringElement(void){}

	CGspringElement(const Vector3d& p0, const Vector3d& p1, const double &vol)
	{
		_init(p0, p1, NULL, vol);
	}

	//construction for an element using nodes coord. in world system and a reference 
	//plane, where the reference plane is represented with its 3 axzs stored in a 3x3 mat
	CGspringElement(const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(p0, p1, &rot, vol);
	}

	const Tensor3x3& getDeformationGradient(void) const
	{	
		static Tensor3x3 r;
		r.IdentityMatrix();
		return r;
	}

	inline void init(const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(p0, p1, &rot, vol);
	}

	inline void computeNodalForce( 
		const Vector3d &TXI,				//node I pos. in world
		const Vector3d &TXJ,				//node J pos. in world
		const CMeMaterialProperty &matril,	//material property for element
		Vector3d &fi,						//elastic force for node i
											//elastic force for node j is Fj = -Fi;
		Tensor3x3 *stiffnessi=NULL			//stiffness matrix
		)
	{
		const Vector3d TXIJ = TXI - TXJ;
		_computeNodalForce(TXIJ, matril, fi, stiffnessi);
	}

	inline void computeNodalForce( 
		const Vector3d &TXI,				//node I pos. in world
		const Vector3d &TXJ,				//node J pos. in world
		const double3x3& rot,				//world-to-local trans. matrix, 3x3 rotational part
		const CMeMaterialProperty &matril,	//material property for element
		Vector3d &forcei,					//elastic force for node i in the world coord. sys.
											//elastic force for node j has Fj = -Fi;
		Tensor3x3 *stiffi=NULL				//stiffness
		)
	{
		const Vector3d dir = TXI - TXJ;		//truss vector
		const Vector3d tmp = m_XI;
		m_XI = m_XI * rot;
		_computeNodalForce(dir, matril, forcei, stiffi);
		m_XI = tmp;
	}

	//output the information of this element
	void exportState(const CMeMaterialProperty &material, FILE *fp) const;

	double computeElementEnergy(const CMeMaterialProperty &matril) const;
};

#endif