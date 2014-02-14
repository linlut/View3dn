//FILE: shear_element.h
//Nan Zhang, Jan 30, 2009

#ifndef _INC_SHEAR_ELM_2009_H_
#define _INC_SHEAR_ELM_2009_H_

#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"

#define Tensor3x3 double3x3
#define ZERO_RODLENGTH 1e-60


//Energey function is (F-I):C:(F-I)
//this method generates better results than the small strain energy
void ComputeNodalForceFinal( 
		const Tensor3x3& F_oldtotal,		//previous total deformation gradient
		const Tensor3x3& F_oldtotal_damp,	//previous total deformation gradient
		const Vector3d &TXI,				//node i's position at current time step
		const Vector3d &XI,					//node i's position at previous time step
		const double& vol,					//volume of the rod
		const CMeMaterialProperty &material,//material property for element
		Tensor3x3& F_total,					//total deformation gradient
		Tensor3x3& F_plastic,				//plastic tensor stored in the elm
		Vector3d *fi,						//elastic force for node i, node j's Fj = -Fi;
		Tensor3x3 *stiffness);				//stiffness matrix

//Chauchy strain
inline void ChauchyStrain(const Tensor3x3& F_total, Tensor3x3& E)
{	
	const double *p = F_total.x;
	double *q = E.x;
	const double ONE=1.0;
	q[0] = p[0]-ONE;
	q[4] = p[4]-ONE;
	q[8] = p[8]-ONE;
	q[1] = q[3] = (p[1]+p[3])*0.5;	
	q[2] = q[6] = (p[2]+p[6])*0.5;
	q[5] = q[7] = (p[5]+p[7])*0.5;
}

//compute force when stress is symetric, 
//0.5*(deriv+deriv^T)**stress = deriv**stress, where ** is inner product
inline void ComputeElasticForce(const Vector3d& deriv, const Tensor3x3& stress, Vector3d& FF)
{
	const Vector3d *r0 = (const Vector3d*)&stress.x[0];
	FF.x = DotProd(deriv, *r0);
	FF.y = DotProd(deriv, r0[1]);
	FF.z = DotProd(deriv, r0[2]);
}

//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: origin (0, 0, 0)
//		TXJ: origin (0, 0, 0)
//		FIJ: incremental deformation gradient - I = pDisp/pX
//----------------------------------------------------------
inline void computeARCDeformationGradientAndDeriv(
	const Vector3d& XI, const Vector3d& TXI, Tensor3x3& FIJ, Vector3d& FIJ_deriv)
{
    const Vector3d DIJ =  TXI - XI;
	const double R2_ROD = Magnitude2(XI)+ 1e-20;
	FIJ_deriv = XI / R2_ROD;
	vectorTensorProduct(DIJ, FIJ_deriv, FIJ);
	FIJ.AddIdentity();
}

//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: origin (0, 0, 0)
//		TXJ: origin (0, 0, 0)
//		FIJ: incremental deformation gradient - I = pDisp/pX
//----------------------------------------------------------
inline void computeWeightedARCDeformationGradientAndDeriv(
	const Vector3d& XI, const Vector3d& TXI, const double &w, 
	Tensor3x3& wFIJ, Vector3d& wFIJ_deriv)
{
    const Vector3d DIJ =  TXI - XI;
	double R2_ROD = Magnitude2(XI) + ZERO_RODLENGTH;
	//a tensor product operation
	wFIJ_deriv = XI *( w / R2_ROD);
	vectorTensorProduct(DIJ, wFIJ_deriv, wFIJ);
}

class CShearElement
{
private:
	Vector3d m_XI;		//Node 0 position in previous time step
						//Node 1 position is always zero in previous time step
	Tensor3x3 m_F;		//Deformation gradient at the previous step

	Tensor3x3 m_EP;		//Plastic strain
	
	double m_vol;		//volume of the truss

private:
	void _init(const Vector3d& p0, const Vector3d& p1, const double3x3* pmat, const double vol)
	{
		if (pmat==NULL)
			m_XI = p0 - p1;
		else
			m_XI = (*pmat)*(p0-p1);
		m_F.IdentityMatrix();
		m_EP.ZeroMatrix();
		m_vol = vol;
	}

	inline void _updateState(const Vector3d& XI, const Tensor3x3& F)
	{
		m_XI = XI;			//position in local coord.
		m_F = F;			//deformation gradient in local coord.
	}
	
	//Energey function is (F-I):C:(F-I)
	//this method generates better results than the small strain energy
	void _computeNodalForce(
			const Tensor3x3& F_oldtotal,		//previous total deformation gradient
			const Tensor3x3& F_oldtotal_damp,	//previous total deformation gradient
			const Vector3d &TXI,				//current position
			const CMeMaterialProperty &material,//material property for element
			Vector3d *fi=NULL,					//elastic force for node i, Fj = - Fi
			Tensor3x3 *stiffnessi=NULL)			//stiffness for node i, if not NULL
	{
		ASSERT0(fi!=NULL);
		Tensor3x3 F_total;
		ComputeNodalForceFinal(F_oldtotal, F_oldtotal_damp, TXI, m_XI, m_vol, material, F_total, m_EP, fi, stiffnessi);
		_updateState(TXI, F_total);
	}

public:

	CShearElement(void)
	{
		m_XI=Vector3d(0,0,0);				
		m_F.IdentityMatrix();
		m_EP.ZeroMatrix();
		m_vol=0;
	}

	CShearElement(const Vector3d& p0, const double &vol)
	{
		const Vector3d p1(0,0,0); 
		_init(p0, p1, NULL, vol);
	}

	CShearElement(const Vector3d& p0, const Vector3d& p1, const double &vol)
	{
		_init(p0, p1, NULL, vol);
	}

	//construction for an element using nodes coord. in world system and a reference 
	//plane, where the reference plane is represented with its 3 axzs stored in a 3x3 mat
	CShearElement(const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(p0, p1, &rot, vol);
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

	inline void init(const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(p0, p1, &rot, vol);
	}

	inline void computeNodalForce( 
			const Vector3d &dir,				//direction vec
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		_computeNodalForce(m_F, m_F, dir, matril, &fi, stiffnessi);
	}

	inline void computeNodalForce( 
			const double3x3& F_total,			//deformation gradient, prev step
			const double3x3& F_total_damp,		//deformation gradient, prev step for damping
			const Vector3d &dir,				//direction vec
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		_computeNodalForce(F_total, F_total_damp, dir, matril, &fi, stiffnessi);
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
			Tensor3x3 *stiffi=NULL)
	{
		const Vector3d TXIJ = rot*dir;					//rotate into the local frame
		Vector3d Flcl;									//force in the local coordinate sys
		//First, local force is computed (in local frame)
		_computeNodalForce(m_F, m_F, TXIJ, matril, &Flcl,stiffi); 
		//Rotate back to global frame. Since mat is rotation mat, mat^(-1)=mat^T
		//No need to compute mat^(-1) explicitly, just use left mult
		forcei = Flcl*rot;								
		//Rotate the stiffness matrix
		if (stiffi){		
			const int mattype = matril.getMaterialType();
			if (mattype==0){
				//Stiffness matrix is constant*Identity. 
				//No need to mult rotation.
			}
			else{
				Tensor3x3 k;								//rotate local stiffness mat: K = R^T * k * R
				matrix3x3TransposeMult(rot, *stiffi, k);	// k = R^T * k
				(*stiffi) = k*rot;							// K = k * rot
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

	static bool isStiffnessMatrix3X3(void){ return false; }

};

#endif