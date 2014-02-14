///FEM attribute list 

#ifndef __OBJ_FEM_ATTRIB_3160_H__
#define __OBJ_FEM_ATTRIB_3160_H__


#include <vector3d.h>


class CObjectFemAttrib
{
public:
	int m_nRigid;				//as rigid as possible? default 0: not rigid, 1: rigid
	int m_nDeformationMethod;	//1: FEM or 0: TRUSS method
	int m_nSpringType;			//We plan to support several types of spring generation:
								// 0: Adaptive, meshfree type of truss
								// 1: Nonadaptive truss, basically 
								// 2: G's method       ---could only be used on tet mesh
								// 3: TVCG 2007 method ---could only be used on tet mesh

	double m_young;				//Young's modulus
	double m_poisson;			//Poisson ratio
	double m_rho;				//Density
	double m_velocity_damp;		//Damping for velocity
	double m_mass_damp;			//Damping for mass

	Vector3d m_vLoad;			//Load
	Vector3d m_vVelocity;		//Initial velocity
	Vector3d m_vDisplacement;	//Initial displacement
    double m_lfShellThickness;  //Shell thickness, only for shell element
	int m_nLayer;				//# of material layers for shell element
	char m_strMatName[40];
	
	//==========================================
	//Internal Data used for output CIA3D
	int m_nAccVertexCount;

public:
	CObjectFemAttrib(void)
	{
		//Simulation
		m_nRigid = 0;
		m_nDeformationMethod = 0;//Using TRUSS in default
		m_nSpringType = 0;		 //Use adaptive meshfree method

		m_young = 1000;			 //Young's modulus, these values are in internation units
		m_poisson = 0.30;		 //Poisson ratio
		m_rho = 1000;			 //Density
		m_velocity_damp = 1e-4;
		m_mass_damp = 1e-3;

		//Initial conditions
        m_vLoad = Vector3d(0,0,0);
		m_vVelocity = Vector3d(0,0,0);
		m_vDisplacement = Vector3d(0,0,0);
		m_strMatName[0] = 0;
		m_nLayer = 1;
        m_lfShellThickness = 0.001; ///1mm default value
	}

	~CObjectFemAttrib()
	{}

	void toString(char* str)
	{
		char nulmat[]="<not set>";
		char *pmatname;
		if (m_strMatName[0]==0) 
			pmatname=nulmat;
		else 
			pmatname=m_strMatName;

		sprintf(str, 
			"Young's modulus %lg, Poisson ratio %lg, Density %lg\n"
			"Load: (%.12lg,%.12lg,%.12lg)\n"
			"Velocity: (%.12lg,%.12lg,%.12lg)\n"
			"Displacement: (%.12lg,%.12lg,%.12lg)\n"
			"Material ID: %s\n", 
			m_young, m_poisson, m_rho,
			m_vLoad.x, m_vLoad.y, m_vLoad.z,
			m_vVelocity.x, m_vVelocity.y, m_vVelocity.z,
			m_vDisplacement.x, m_vDisplacement.y, m_vDisplacement.z,
			pmatname);
	}

};


#define CFemObjAttrib CObjectFemAttrib

#endif