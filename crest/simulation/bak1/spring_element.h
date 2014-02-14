//FILE: spring_element.h

#ifndef _INC_SPRING_ELM_ZNH_
#define _INC_SPRING_ELM_ZNH_


#include <vector3d.h>
#include "mechanical_property.h"

class CSpringElement
{
private:
	double m_restLength;	//rest length of the spring
	double m_K;				//stiff ratio
	double m_Kd;			//stiff ratio for damping
	int m_vertexID[2];		//vertex ids for the two nodes

private:
	void _init(const Vector3d& p0, const Vector3d& p1);

	void _computeElasticForce(const Vector3d &p0, const Vector3d &p1, Vector3d &force, Vector3d &Xij);


public:
	void init(const int vertexid[2], const Vector3d& p0, const Vector3d& p1, const double &K, const double &Kd)
	{
		m_vertexID[0] = vertexid[0];
		m_vertexID[1] = vertexid[1];
		m_K = K;
		m_Kd = Kd;
		_init(p0, p1);
	}

	CSpringElement(void){}

	CSpringElement(const int vertexid[2], const Vector3d& p0, const Vector3d& p1, const double &K, const double &Kd=0)
	{
		init(vertexid, p0, p1, K, Kd);
	}

	const int *getElementNodeBuffer(void)
	{
		return m_vertexID;
	}

	const double3x3* getDeformationGradient(void) const
	{
		return NULL;
	}

	void computeNodalForce( const Vector3d &p0,				//node 0's world position
							const Vector3d &p1,				//node 1's world position
							Vector3d &f0,					//elastic force for node 0
							double3x3 *stiffnessi=NULL
							);

	//if damping is required, we compute additional damping force in the 
	//returned force vector
	void computeNodalForce( const Vector3d &p0,				//node 0's world position
							const Vector3d &p1,				//node 1's world position
							const Vector3d &v0,				//node 0's velocity
							const Vector3d &v1,				//node 1's velocity
							Vector3d &f0,					//elastic force for node 0
							double3x3 *stiffnessi=NULL
							);

};


#endif