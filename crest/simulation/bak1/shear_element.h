//FILE: shear_element.h

#ifndef _INC_SHEAR_ELM_H_
#define _INC_SHEAR_ELM_H_

#include <vectorall.h>
#include <matrix3x3.h>
#include "mechanical_property.h"

#define Tensor3x3 double3x3


//Input matrix A, B, output C, performs C=A^T*B
inline void Matrix3x3TransposeMult(const double3x3& MA, const double3x3& MB, double3x3& MC)
{
	const double *A=MA.x;
	const double *B=MB.x;
	double *C = MC.x;
	//first row
	C[0] = A[0]*B[0] + A[3]*B[3] + A[6]*B[6];  
	C[1] = A[0]*B[1] + A[3]*B[4] + A[6]*B[7];  
	C[2] = A[0]*B[2] + A[3]*B[5] + A[6]*B[8];  
	//second row
	C[3] = A[1]*B[0] + A[4]*B[3] + A[7]*B[6];  
	C[4] = A[1]*B[1] + A[4]*B[4] + A[7]*B[7];  
	C[5] = A[1]*B[2] + A[4]*B[5] + A[7]*B[8];  
	//third row
	C[6] = A[2]*B[0] + A[5]*B[3] + A[8]*B[6];  
	C[7] = A[2]*B[1] + A[5]*B[4] + A[8]*B[7];  
	C[8] = A[2]*B[2] + A[5]*B[5] + A[8]*B[8];
}


class CShearElement
{
private:
	Vector3d m_XI;		//Node 0 position in previous time step
						//Node 1 position is always zero in previous time step
	Vector3d m_force;	//Elastic force 
	Tensor3x3 m_F;		//Deformation gradient at the previous step
	
	//--------------------------------------------------------
	//Vector3d m_TXI;	//Node 0 position in current time step	
						//Node 1 position in current time step (always zero)
	//Tensor3x3 m_TF;	//Deformation gradient tensor at current step
	double m_vol;		//volume of the truss

private:
	void _init(const Vector3d& p0, const Vector3d& p1, const double3x3* pmat, const double &vol)
	{
		if (pmat==NULL)
			m_XI = p0 - p1;
		else
			m_XI = (*pmat)*(p0-p1);
		m_F.IdentityMatrix();
		m_vol = vol;
		m_force = Vector3d(0,0,0);
	}

	inline void _shrinkTensor(const Tensor3x3 &F)
	{
		const double *x = F.x;
		const double k1 = fabs(x[0]+x[4]+x[8]-3)*1e-3;
		const double k = 1-k1;
		const double X0 = F.x[0]*k + k1;
		const double X4 = F.x[4]*k + k1;
		const double X8 = F.x[8]*k + k1;
		m_F.x[0] = X0;
		m_F.x[4] = X4;
		m_F.x[8] = X8;
		m_F.x[1] = F.x[1]*k;		
		m_F.x[2] = F.x[2]*k;		
		m_F.x[3] = F.x[3]*k;		
		m_F.x[5] = F.x[5]*k;		
		m_F.x[6] = F.x[6]*k;		
		m_F.x[7] = F.x[7]*k;		
	}

	inline void _updateState(const Vector3d& XI, const Tensor3x3& F, const Vector3d *force=NULL)
	{
		m_XI = XI;			//position in local coord.
		m_F = F;			//deformation gradient in local coord.
		if (force) m_force = *force;
		return;
		//_shrinkTensor(F);
	}

	inline void _updateState(const Vector3d& XI, const Vector3d &force)
	{
		m_XI = XI;			//position in local coord.
		m_force = force;	//force in local coord.
	}

	//elastic force for node i
	//elastic force for node j is Fj = -Fi;
	//stiffness matrix for node i, if the pointer is not NULL
	void _computeNodalForce(const Vector3d &TXI, const CMeMaterialProperty &matril, 
			Vector3d *fi=NULL, Tensor3x3 *stiffnessi=NULL);

	void _computeNodalForceN(const Vector3d &TXI, const CMeMaterialProperty &matril, 
			Vector3d *fi=NULL, Tensor3x3 *stiffnessi=NULL);

	void _computeNodalForceI(const Vector3d &TXI, const CMeMaterialProperty &matril, 
			Vector3d *fi=NULL, Tensor3x3 *stiffnessi=NULL);

	inline void _computeNodalForceLargeSteps( 
			const Vector3d &TXIJ,				//node I pos. in world
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		const double TH0 = 0.05;
		const double TH = TH0*TH0;
		const double TH_INV = 1.0/TH;
		const Vector3d difvec = TXIJ - m_XI;
		const double len1 = Magnitude2(difvec);
		const double len0 = Magnitude2(m_XI);
		const double dif = fabs(len1/len0);
		if (dif<TH){
			_computeNodalForceN(TXIJ, matril, &fi, stiffnessi);
		}
		else{
			int N = (int)(sqrtf(dif*TH_INV) + 1.50f);	//sqrtf is single precision, faster
			const Vector3d stepsize = difvec/N;
			Vector3d cpos = m_XI;
			for (int i=0; i<N-1; i++){
				cpos+=stepsize;
				_computeNodalForceN(cpos, matril);
			}
			cpos+=stepsize;
			_computeNodalForceN(cpos, matril, &fi, stiffnessi);
			//printf("Adaptive step size: N =___________%d\n", N);
		}
	}

public:

	CShearElement(void){}

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

	inline void init(const Vector3d& p0, const Vector3d& p1, const double3x3& rot, const double &vol)
	{
		_init(p0, p1, &rot, vol);
	}

	inline void computeNodalForce( 
			const Vector3d &TXI,				//node I pos. in world
			const Vector3d &TXJ,				//node J pos. in world
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &fi,						//elastic force for node i, node j is Fj = -Fi;
			Tensor3x3 *stiffnessi=NULL)			//stiffness matrix
	{
		const Vector3d TXIJ = TXI - TXJ;
		_computeNodalForceLargeSteps(TXIJ, matril, fi, stiffnessi);
	}

	inline void computeNodalForce( 
			const Vector3d &TXI,				//node I pos. in world
			const Vector3d &TXJ,				//node J pos. in world
			const double3x3& rot,				//world-to-local transform matrix, 3x3 rotational part
			const CMeMaterialProperty &matril,	//material property for element
			Vector3d &forcei,					//elastic force for node i in the world coord. sys., node j has Fj = -Fi;
			Tensor3x3 *stiffi=NULL)
	{
		const Vector3d dir = TXI - TXJ;					//truss vector
		const Vector3d TXIJ = rot*dir;					//rotate into the local frame
		Vector3d Flcl;									//force in the local coordinate sys
		//First, local force is computed (in local frame)
		//_computeNodalForceLargeSteps(TXIJ, matril, Flcl,stiffi); 
		_computeNodalForceN(TXIJ, matril, &Flcl,stiffi); 
		//Rotate back to global frame, since mat is a rotation matrix, mat^(-1)=mat^T
		//There is no need to compute it explicitly, just left mult
		forcei = Flcl*rot;								
		//Roate the stiffness matrix
		if (stiffi){								
			Tensor3x3 k;								//rotate local stiffness mat: K = R^T * k * R
			Matrix3x3TransposeMult(rot, *stiffi, k);	// k = R^T * k
			(*stiffi) = k*rot;							// K = k * rot
		}	
	}

	//output the information of this element
	void exportState(const CMeMaterialProperty &material, FILE *fp) const;

	double computeElementEnergy(const CMeMaterialProperty &matril) const;
};

#endif