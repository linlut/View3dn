#include <sysconf.h>
#include <vectorall.h>
#include <matrix3x3.h>
#include <view3dn/scenegraph.h>

#define Tensor3x3 double3x3

struct CSimmuNode{
private:
	Vector3d m_force;				//Nodal force
	Vector3d m_acceleration;		//Acceleration
	Vector3d m_velocity;			//Velocity of the node

public:
	Vector3d m_pos;



	void initBeforeEachIteration(void)
	{
		m_force.x = m_force.y = m_force.z = 0;
	}

};


struct CSimmuElement
{
private:
	Tensor3x3 m_Ftn;					//Deformation gradient at the previous step

public:
	Vector3i m_elm;
	const unsigned char m_cElmType;		//element type
	const unsigned char m_cMatType;		//material type


};