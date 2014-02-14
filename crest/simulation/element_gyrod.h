//FILE: element_gyrod.h
//Nan Zhang, Jan 30, 2009

#ifndef _INC_ELM_GYROD_JAN2009_H_
#define _INC_ELM_GYROD_JAN2009_H_

#include <vectorall.h>
#include <matrix3x3.h>
#include "element_shear.h"
#include "gyrod_lut.h"

class CGyrodLookupTable;

class CGyrodElement
{
private:
	double3x3 m_R;				//rotation matrix for the rod to be concide with the Z axis
	double m_len0;				//initial rod length
	CShearElement m_shearelm;	//based on shear element

	//a global lookup table is used for large steps, where a cubemap for acc.
	static CGyrodLookupTable* m_pLookupTable;

private:
	void _init(const Vector3d& p, const double3x3& mat, const double &vol);

	//the current method used in the system, use a simple strain def. \epsilon = F - I
	inline void _computeNodalForce(
		const Vector3d& TXI,				//node i position
		const CMeMaterialProperty& material,//material property for element
		Vector3d* fi,						//elastic force for node i, elastic force for node j is Fj = -Fi;
		Tensor3x3* stiffness)				//stiffness matrix
	{
		//First, use the CubeMapping method to search the reference point
		const CLookupTableElement *pelm = m_pLookupTable->findLookupTableElementCubeMap(TXI);	
		const double3x3& dampingDG = *(m_shearelm.getDeformationGradient());

		//Then, check edge length, approximate when TXI and m_dir are almost parallel
		Vector3d refvert;
		double3x3 *pDG, currentDG;
		const double edgelen = DotProd(TXI, pelm->m_dir); 
		const double K = edgelen/m_len0;
		const double TOL1 = 0.97, TOL2 = 1.03;  
		if (K>TOL1 && K<TOL2){
			pDG = (double3x3*)&pelm->m_F;
			refvert = pelm->m_dir*m_len0;
		}
		else{
			VectorTensorProduct(pelm->m_dir, (K-1.0)*pelm->m_dir, currentDG);
			currentDG.AddIdentity();
			currentDG *= pelm->m_F;
			pDG = &currentDG;
			refvert = pelm->m_dir*edgelen; 
		}

		//real computation
		m_shearelm.setLocalPosition(refvert); 
		m_shearelm.computeNodalForce(*pDG, dampingDG, TXI, material, *fi, stiffness);
	}

public:

	CGyrodElement(void){}

	CGyrodElement(const Vector3d& p, const double3x3& mat, const double& vol)
	{
		_init(p, mat, vol);
	}

	CGyrodElement(const Vector3d& p0, const Vector3d &p1, const double& vol)
	{
		double3x3 mat; mat.setIdentityMatrix();
		const Vector3d p = p0 - p1;
		_init(p, mat, vol);
	}

	inline double getVolume(void) const
	{
		return m_shearelm.getVolume();
	}

	inline void init(const CMeMaterialProperty &mtl, 
		const Vector3d& p, const double3x3& mat, const double &vol)
	{
		_init(p, mat, vol);
	}

	inline void init(
		const CMeMaterialProperty &mtl,
		const Vector3d &p0, const Vector3d &p1, const double3x3& mat, const double &vol)
	{
		const Vector3d p = p0 - p1;
		_init(p, mat, vol);
	}

	inline void computeNodalForce( 
			const Vector3d& dir,				//direction vec
			const CMeMaterialProperty& matril,	//material property for element
			Vector3d& fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3* stiffnessi=NULL)			//stiffness matrix
	{
		Vector3d ff;
		const Vector3d p= m_R * dir;
		_computeNodalForce(p, matril, &ff, stiffnessi);
		fi = ff * m_R;
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
			const double3x3& _rot,				//world-to-local transform matrix, 3x3 rotational part
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &forcei,					//elastic force for node i in the world coord. sys., node j has Fj = -Fi;
			Tensor3x3 *stiffi=NULL)
	{
		const double3x3 rot = m_R * _rot;
		const Vector3d TXIJ = rot*dir;					//rotate into the local frame
		Vector3d Flcl;									//force in the local coordinate sys
		//First, local force is computed (in local frame)
		_computeNodalForce(TXIJ, matril, &Flcl,stiffi); 
		//Rotate back to global frame, since mat is a rotation matrix, mat^(-1)=mat^T
		//There is no need to compute it explicitly, just left mult
		forcei = Flcl*rot;								
		//Roate the stiffness matrix
		if (stiffi){		
			const int mattype = matril.getMaterialType();
			if (mattype==0){
				//Stiffness matrix is constant*Identity. 
				//No need to mult rotation.
			}
			else{
				Tensor3x3 k;								//rotate local stiffness mat: K = R^T * k * R
				Matrix3x3TransposeMult(rot, *stiffi, k);	// k = R^T * k
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

	inline void exportState(const CMeMaterialProperty &material, FILE *fp) const
	{
		//m_shearelm.exportState(material, fp);
	}

	inline const double3x3* getDeformationGradient(void) const
	{
		return m_shearelm.getDeformationGradient();
	}

	inline double3x3* getDeformationGradient(void)
	{
		return m_shearelm.getDeformationGradient();
	}

	static inline CGyrodLookupTable* getLookupTable(void)
	{
		return m_pLookupTable;
	}

	static void initLookupTable(void);

	static bool hasConstantStiffness(void){ return false; }

	static bool isStiffnessMatrix3X3(void){ return false; }

};

#endif