//FILE: shear_simp_element.h
//Nan Zhang, Dec 18, 2010

#ifndef _INC_SIMPSHEAR_ELM_2010_H_
#define _INC_SIMPSHEAR_ELM_2010_H_

#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"
#include "element_shear.h"


class CSimpShearElement
{
private:
	Vector3d m_XI;		//Node 0 position in time 0
						//Node 1 position is always zero in previous time step
	Tensor3x3 m_F;		//Deformation gradient at the previous step

	Tensor3x3 m_stiff;	//Plastic strain

	Tensor3x3 m_EP;		//Plastic strain
	
	double m_vol;		//volume of the truss

private:
	void _init(const CMeMaterialProperty &mtl, 
		const Vector3d& p0, const Vector3d& p1, const double3x3* pmat, const double& vol)
	{
		if (pmat==NULL)
			m_XI = p0 - p1;
		else
			m_XI = (*pmat)*(p0-p1);
		m_F.setIdentityMatrix();
		m_EP.setZeroMatrix();
		m_stiff.setZeroMatrix();
		m_vol = vol;

		//init stiffness matrix
		_computeStiffness(vol, mtl);
	}

	inline void _updateState(const Tensor3x3& F)
	{
		m_F = F;  //previous time step deformation gradient, in local coord.
	}

	void __ComputeNodalForceFinal2( 
			Tensor3x3& F_total,					//total deformation gradient
			Tensor3x3& F_plastic,				//plastic tensor stored in the elm
			const Vector3d &TXI,				//node i's position at current time step
			const Vector3d &XI,					//node i's position at previous time step
			const double& vol,					//volume of the rod
			const CMeMaterialProperty &material,//material property for element
			Vector3d *fi,						//elastic force for node i, node j's Fj = -Fi;
			Tensor3x3 *stiffness);				//stiffness matrix

	//Energey function is (F-I):C:(F-I)
	//this method generates better results than the small strain energy
	inline void _computeNodalForce(
			const Vector3d &TXI,				//current position
			const CMeMaterialProperty &material,//material property for element
			Vector3d *fi=NULL,					//elastic force for node i, Fj = - Fi
			Tensor3x3 *stiffnessi=NULL)			//Hessian matrix for node i, if not NULL
	{
		ASSERT0(fi!=NULL);
		__ComputeNodalForceFinal2(m_F, m_EP, TXI, m_XI, m_vol, material, fi, stiffnessi);
	}

	//init the stiffness matrix
	void _computeStiffness(const double& vol, const CMeMaterialProperty &mtl);	

public:
	CSimpShearElement(void)
	{
		m_XI = Vector3d(1,0,0);				
		m_F.setIdentityMatrix();
		m_EP.setZeroMatrix();
		m_vol=0;
	}

	/*
	CSimpShearElement(const Vector3d& p0, const double &vol)
	{
		const Vector3d p1(0,0,0); 
		_init(p0, p1, NULL, vol);
	}
	*/

	CSimpShearElement(const CMeMaterialProperty &mtl, 
		const Vector3d& p0, const Vector3d& p1, const double &vol)
	{
		_init(mtl, p0, p1, NULL, vol);
	}
	

	//construction for an element using nodes coord. in world system and a reference 
	//plane, where the reference plane is represented with its 3 axzs stored in a 3x3 mat
	CSimpShearElement(
		const CMeMaterialProperty &mtl, 
		const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(mtl, p0, p1, &rot, vol);
	}

	inline const Tensor3x3* getDeformationGradient(void) const
	{
		return &m_F;
	}

	inline Tensor3x3* getDeformationGradient(void)
	{
		return &m_F;
	}

	inline Vector3d& getLocalPosition(void)
	{
		return m_XI;
	}

	inline const Vector3d& getLocalPosition(void) const
	{
		return m_XI;
	}

	inline void setLocalPosition(const Vector3d& pos)
	{
		m_XI = pos;
	}

	inline void setDeformationGradient(const Tensor3x3& F)
	{
		m_F = F;
	}

	inline double getVolume(void) const
	{
		return m_vol;
	}

	inline void init(const CMeMaterialProperty &mtl, 
		const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(mtl, p0, p1, &rot, vol);
	}

	inline void computeNodalForce( 
			const Vector3d &dir,				//direction vec
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		_computeNodalForce(dir, matril, &fi, stiffnessi);
	}

	inline void computeNodalForce( 
			const Vector3d &TXI,				//node I pos. in world
			const Vector3d &TXJ,				//node J pos. in world
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		const Vector3d TXIJ = TXI - TXJ;
		computeNodalForce(TXIJ, matril, fi, stiffnessi);
	}

	inline void computeNodalForce( 
			const Vector3d& dir,				//node I pos. in world
			const double3x3& rot,				//world-to-local transform matrix, 3x3 rotational part
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &forcei,					//elastic force for node i in the world coord. sys., node j has Fj = -Fi;
			Tensor3x3 *stiff=NULL)
	{
		//First, local force is computed (in local frame)
		Vector3d TXIJ = rot*dir;				//rotate into the local frame
		Vector3d Flcl;							//local force 
		_computeNodalForce(TXIJ, matril, &Flcl, stiff); 

		//Rotate back to global frame. Since mat is rotation mat, mat^(-1)=mat^T
		//No need to compute mat^(-1) explicitly, just use left mult
		forcei = Flcl*rot;								

		//Rotate the stiffness matrix
		if (stiff){		
			if (matril.getMaterialType()==0){
				//zero rest length spring type material
				//Stiffness matrix is constant*Identity. 
				//No need to mult rotation.
			}
			else{//material can support both Young's modulus and Poisson ratio
				Tensor3x3 k;							//rotate local stiff: K = R^T * k * R
				Matrix3x3TransposeMult(rot, *stiff, k); // k = R^T * k
				(*stiff) = k*rot;						// K = k * rot
			}
		}	
	}

	inline void computeNodalForce( 
			const Vector3d &TXI,				//node I pos. in world
			const Vector3d &TXJ,				//node J pos. in world
			const double3x3& rot,				//world-to-local transform matrix, 3x3 rotational part
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &forcei,					//elastic force for node i in the world coord. sys., node j has Fj = -Fi;
			Tensor3x3 *stiffi=NULL)
	{
		const Vector3d dir = TXI - TXJ;
		computeNodalForce(dir, rot, matril, forcei, stiffi); 
	}

	//output the information of this element
	void exportState(const CMeMaterialProperty &material, FILE *fp) const;

	double computeElementEnergy(const CMeMaterialProperty &matril) const;

	static bool hasConstantStiffness(void){ return false; }

	static bool isStiffnessMatrix3X3(void){ return true; }

	//compute the symbolic Jacobian
	void computeJacobianSymbolic(const CMeMaterialProperty* pmtl, double3x3& lambdaMat, double3x3& muMat);

};

#endif