//FILE: shear_element.cpp
#include "shear_element.h"

// C = A+B, where A and B are both symetric matrices
static inline Tensor3x3 SymetricAPlusSymetricB(const Tensor3x3& A, const Tensor3x3 &B)
{
	Tensor3x3 C;
	const double *p = A.x;
	const double *q = B.x;
	double *x = C.x;
	x[0] = p[0]+q[0]; x[1] = p[1]+q[1]; x[2] = p[2]+q[2];
	x[4] = p[4]+q[4]; x[5] = p[5]+q[5];
	x[8] = p[8]+q[8];
	x[3]=x[1];
	x[6]=x[2];
	x[7]=x[5];
	return C;
}

// C = A*B
//The first row of A is non zero, the rest two rows are zero
static inline Tensor3x3 multMatARow0MatB(const Tensor3x3& A, const Tensor3x3 &B)
{
	Tensor3x3 C;
	const double *p = A.x;
	const double *q = B.x;
	double *x = C.x;
	x[0] = p[0]*q[0]+p[1]*q[3]+p[2]*q[6];
	x[1] = p[0]*q[1]+p[1]*q[4]+p[2]*q[7];
	x[2] = p[0]*q[2]+p[1]*q[5]+p[2]*q[8];
	x[3]=x[4]=x[5]=x[6]=x[7]=x[8]=0;
	return C;
}

//The tensor product operation between two 1D vectors
static inline void vectorTensorProduct(const Vector3d &DIJ, const Vector3d &N, Tensor3x3 &FIJ)
{
	double *x = FIJ.x;
	x[0] = DIJ.x * N.x;
	x[1] = DIJ.x * N.y;
	x[2] = DIJ.x * N.z;
	x[3] = DIJ.y * N.x;
	x[4] = DIJ.y * N.y;
	x[5] = DIJ.y * N.z;
	x[6] = DIJ.z * N.x;
	x[7] = DIJ.z * N.y;
	x[8] = DIJ.z * N.z;
}

static inline double SymetricTensorDotProd(const Tensor3x3& A, const Tensor3x3& B)
{
	const double *p = A.x;
	const double *q = B.x;
	const double r1 = p[0]*q[0]+p[4]*q[4]+p[8]*q[8];
	const double r2 = p[1]*q[1]+p[2]*q[2]+p[5]*q[5];
	return r1+r2+r2;
}

//Chauchy strain
static inline void ChauchyStrain(const Tensor3x3& F_total, Tensor3x3& E)
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

static inline void ChauchyStrainSmall(const Tensor3x3& F_total, Tensor3x3& E)
{	
	const double *p = F_total.x;
	double *q = E.x;
	const double ONE=1.0;
	q[0] = p[0];
	q[4] = p[4];
	q[8] = p[8];
	q[1] = q[3] = (p[1]+p[3])*0.5;	
	q[2] = q[6] = (p[2]+p[6])*0.5;
	q[5] = q[7] = (p[5]+p[7])*0.5;
}

//Chauchy strain rate
static inline void ChauchyStrainRate(const Tensor3x3& F_total, Tensor3x3& E)
{	
	const double *p = F_total.x;
	double *q = E.x;
	q[0] = p[0];
	q[4] = p[4];
	q[8] = p[8];
	q[1] = q[3] = (p[1]+p[3])*0.5;
	q[2] = q[6] = (p[2]+p[6])*0.5;
	q[5] = q[7] = (p[5]+p[7])*0.5;
}

//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: 
//		TXJ:
//		FIJ: incremental deformation gradient
//		R2_ROD: length^2 at t_i
//----------------------------------------------------------
inline void computeARCDeformationGradient( 
		const Vector3d& XI, const Vector3d& TXI, 
		const Vector3d& XJ, const Vector3d& TXJ, 
		Tensor3x3& FIJ,
		Vector3d& XIJ,
		double& R2_ROD)
{
	XIJ = XI - XJ;
	const Vector3d TXIJ = TXI - TXJ;
    const Vector3d DIJ =  TXIJ - XIJ;	
	R2_ROD = Magnitude2(XIJ);
	//a tensor product operation
	const double k = 1.0/R2_ROD;
	XIJ*=k;
	vectorTensorProduct(DIJ, XIJ, FIJ);
	//add the identity tensor
	FIJ[0]+=1.0; FIJ[4]+=1.0; FIJ[8]+=1.0;
}

//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: zero
//		TXJ: zero
//		FIJ: incremental deformation gradient - I = pDisp/pX
//----------------------------------------------------------
inline void 
computeARCDeformationGradientAndDeriv(
	const Vector3d& XI, const Vector3d& TXI, 
	Tensor3x3& FIJ, Vector3d &FIJ_deriv, double &R2_ROD)
{
    const Vector3d DIJ =  TXI - XI;
	R2_ROD = Magnitude2(XI);
	//a tensor product operation
	const double k = 1.0/R2_ROD;
	FIJ_deriv = XI *k;
	vectorTensorProduct(DIJ, FIJ_deriv, FIJ);
}

//Compute the total deformation gradient F and the total deformation gradient rate vs time dF/dt
//note that the rate here lacks the division of dt yet
inline void 
ComputeDeformationGradient(
	const Vector3d &XI, const Vector3d &TXI, const Tensor3x3 &F_oldtotal, 
	Tensor3x3 &F_total, Tensor3x3 &F_total_rate, Vector3d &arcc_deriv)
{
	Tensor3x3 F_arc;
	double R2_ROD;

	//comp. incremental deformation gradient
	computeARCDeformationGradientAndDeriv(XI, TXI, F_arc, arcc_deriv, R2_ROD);
	//comp. the total deformation gradient
	F_total_rate = F_arc * F_oldtotal;
	F_total = F_oldtotal + F_total_rate;
}


//----------------------------------------------------------
//Compute the deriv of the ARC deformation gradient with respect to the 
//delta of incremental displacement DIJ
//Notice this requres XIJ to be divided by R2_ROD already
//----------------------------------------------------------
inline void ComputeARCDeformationGradientDeriv(const Vector3d& XIJ, Vector3d& deriv)
{
	deriv = XIJ;	
	//x[0..2]=XIJ
	//x[3..8]=zero, here we don't need to set them since we use accelerated computation. 
	//Bbe careful when you indeed to use it as a full 3x3 tensor
}

/*
inline void ComputeDeformationGradientDeriv(const Tensor3x3& F_arc_deriv, const Tensor3x3& F_tn, Tensor3x3 F[3])
{
	F[0] = multMatARow0MatB(F_arc_deriv, F_tn);
	F[1].ZeroMatrix(); 
	F[2].ZeroMatrix();
	const double *x = F[0].x;
	double *f1 = F[1].x;
	double *f2 = F[2].x;
	f1[3]=x[0]; f1[4]=x[1]; f1[5]=x[2];
	f2[6]=x[0]; f2[7]=x[1]; f2[8]=x[2];
}
*/


inline void ComputeElasticForce(const Vector3d& deriv, const Tensor3x3& stress, Vector3d &FF)
{
	const Vector3d *r0 = (const Vector3d*)&stress.x[0];
	const Vector3d *r1 = (const Vector3d*)&stress.x[3];
	const Vector3d *r2 = (const Vector3d*)&stress.x[6];
	FF.x = DotProd(deriv, *r0);
	FF.y = DotProd(deriv, *r1);
	FF.z = DotProd(deriv, *r2);
}

inline void ComputeStressDeriv(
		const Vector3d& dg_deriv, 
		const double &Lambda, const double &Miu, 
		const double &LambdaDamp, const double &MiuDamp, 
		Tensor3x3 stress_deriv[3])
{
	Tensor3x3 f[3];
	double *p0 = f[0].x;
	double *p1 = f[1].x;
	double *p2 = f[2].x;

	p0[0] = dg_deriv.x; p0[1] = dg_deriv.y*0.5; p0[2] = dg_deriv.z*0.5;
	p0[3] = p0[1]; p0[4] = 0; p0[5] = 0; 
	p0[6] = p0[2]; p0[7] = 0; p0[8] = 0;

	p1[0] = 0; p1[1] = dg_deriv.x*0.5; p1[2] = 0;
	p1[3] = p1[1]; p1[4] = dg_deriv.y; p1[5] = p0[2];
	p1[6] = 0; p1[7] = p1[5]; p1[8] = 0;

	p2[0] = 0; p2[1] = 0; p2[2] = p1[1];
	p2[3] = 0; p2[4] = 0; p2[5] = p0[1];
	p2[6] = p1[1]; p2[7] = p0[1]; p2[8] = dg_deriv.z;

	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, f[0], stress_deriv[0]);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, f[1], stress_deriv[1]);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, f[2], stress_deriv[2]);

	if (LambdaDamp>0){
		Tensor3x3 stress_deriv_damp[3];		//deriv. for stress pEpsilon/pX
		CMeMaterialProperty::IsotropicMaterialStress(LambdaDamp, MiuDamp, f[0], stress_deriv_damp[0]);
		CMeMaterialProperty::IsotropicMaterialStress(LambdaDamp, MiuDamp, f[1], stress_deriv_damp[1]);
		CMeMaterialProperty::IsotropicMaterialStress(LambdaDamp, MiuDamp, f[2], stress_deriv_damp[2]);
		stress_deriv[0] += stress_deriv_damp[0];
		stress_deriv[1] += stress_deriv_damp[1];
		stress_deriv[2] += stress_deriv_damp[2];
	}
}


//Energy function is used as small strain energy function \epsilon = 0.5*(F+F^T)
//Then, the energy function is defined as normal E = \epsilon : C* \epsilon
//Erros seem to be large
void CShearElement::_computeNodalForce( 
		const Vector3d &TXI,				//node i position
		const CMeMaterialProperty &material,//material property for element
		Vector3d *fi,						//elastic force for node i
											//elastic force for node j is Fj = -Fi;
		Tensor3x3 *stiffness				//stiffness matrix
	    )
{
	Tensor3x3 F_total_rate, strain, stress;
	Vector3d F_arc_deriv;					//arc deformation gradient deriv
	Tensor3x3 straind;						//strain for damping
	Tensor3x3 stressd;						//viscos damping stress
	Vector3d deriv;							//total deformation gradient deriv
	Tensor3x3 stress_deriv[3];				//deriv. for stress pEpsilon/pX
	Tensor3x3 F_total ;						//total defomration gradient

	//deformation gradient F_total, rate of F_total, and derivitive of F_arc
	ComputeDeformationGradient(m_XI, TXI, m_F, F_total, F_total_rate, F_arc_deriv);
	//total strain tensor and stress
	if (fi){
		ChauchyStrain(F_total, strain);
		//assume isotropic material at this time, compute the stress
		//where stress = elastic stress + viscous stress
		double Lambda, Miu, LambdaDamp, MiuDamp;
		material.getLameCofficients(Lambda, Miu);
		const double vol1 = -m_vol;
		Lambda*=vol1; Miu*=vol1;
		CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, strain, stress);
		//compute damping
		material.getLameCofficientsDamping(LambdaDamp, MiuDamp);
		if (LambdaDamp>0){
			ChauchyStrainRate(F_total_rate, straind);
			LambdaDamp*=vol1; MiuDamp*=vol1;
			CMeMaterialProperty::IsotropicMaterialStress(LambdaDamp, MiuDamp, straind, stressd);
			stress+=stressd;
		}
		//Compute DG derivitive matrix, then the final force on node XI
		deriv = dotProdMatTranspose(F_arc_deriv, m_F);
		ComputeElasticForce(deriv, stress, *fi);
		//If stiffness matrix is required
		if (stiffness){	
			ComputeStressDeriv(deriv, Lambda, Miu, LambdaDamp, MiuDamp, stress_deriv);
			Vector3d* row0 = (Vector3d*)(&stiffness->x[0]);
			//stiffness matrix is symetric
			ComputeElasticForce(deriv, stress_deriv[0], *row0); row0++;
			ComputeElasticForce(deriv, stress_deriv[1], *row0); row0++;
			ComputeElasticForce(deriv, stress_deriv[2], *row0); 
		}
	}

	//update the stored var.
	_updateState(TXI, F_total);
}


//Energey function is F^T:C*F
//this method generates better results than the small strain energy
void CShearElement::_computeNodalForceN( 
		const Vector3d &TXI,				//node i position
		const CMeMaterialProperty &material,//material property for element
		Vector3d *fi,						//elastic force for node i
											//elastic force for node j is Fj = -Fi;
		Tensor3x3 *stiffness				//stiffness matrix
	    )
{
	Tensor3x3 F_total_rate, strain, stress;
	Vector3d F_arc_deriv;					//arc deformation gradient deriv
	Tensor3x3 straind;						//strain for damping
	Tensor3x3 stressd;						//viscos damping stress
	Vector3d deriv;							//total deformation gradient deriv
	Tensor3x3 stress_deriv[3];				//deriv. for stress pEpsilon/pX
	Tensor3x3 F_total ;						//total defomration gradient

	//deformation gradient F_total, rate of F_total, and derivitive of F_arc
	ComputeDeformationGradient(m_XI, TXI, m_F, F_total, F_total_rate, F_arc_deriv);
	//total strain tensor and stress
	if (fi){
		stress = F_total; 
		stress.x[0]-=1; stress.x[4]-=1; stress.x[8]-=1;
		//Compute DG derivitive matrix, then the final force on node XI
		deriv = dotProdMatTranspose(F_arc_deriv, m_F);
		//===========================================================		
		//check damping
		const double youngDamp = material.getYoungDamp();
		if (youngDamp>0){
			//double dt, dt_inv2;
			//material.getTimeStep(dt, dt_inv2);
			stressd = youngDamp * F_total_rate;
			stress += stressd;
		}
		//===========================================================
		//compute nodal force
		*fi = dotProd(deriv, stress);
		const double young = material.getYoung();
		*fi *= (-m_vol * young);
	}

	//update the stored var.
	_updateState(TXI, F_total, fi);
}


void CShearElement::_computeNodalForceI( 
		const Vector3d &TXI,				//node i position
		const CMeMaterialProperty &material,//material property for element
		Vector3d *fi,						//elastic force for node i
											//elastic force for node j is Fj = -Fi;
		Tensor3x3 *stiffness				//stiffness matrix
	    )
{
	Tensor3x3 F_total_rate, strain, stress;
	Vector3d F_arc_deriv;					//arc deformation gradient deriv
	Tensor3x3 straind;						//strain for damping
	Tensor3x3 stressd;						//viscos damping stress
	Vector3d deriv;							//total deformation gradient deriv
	Tensor3x3 stress_deriv[3];				//deriv. for stress pEpsilon/pX
	Tensor3x3 F_total ;						//total defomration gradient

	//deformation gradient F_total, rate of F_total, and derivitive of F_arc
	ComputeDeformationGradient(m_XI, TXI, m_F, F_total, F_total_rate, F_arc_deriv);
	//total strain tensor and stress
	if (fi){
		stress = F_total; 
		stress.x[0]-=1; stress.x[4]-=1; stress.x[8]-=1;
		//Compute DG derivitive matrix, then the final force on node XI
		deriv = dotProdMatTranspose(F_arc_deriv, m_F);
		//compute nodal force
		*fi = dotProd(deriv, stress);
		const double young = material.getYoung();
		*fi *= (-young * m_vol);
	}

	//update the stored var.
	_updateState(TXI, F_total);
}


double CShearElement::computeElementEnergy(const CMeMaterialProperty &material) const
{
	/*
	//symetric strain/stress
	Tensor3x3 strain, stress;
	const Tensor3x3 F = m_F;

	ChauchyStrain(F, strain);	
	double Lambda, Miu;
	material.getLameCofficients(Lambda, Miu);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, strain, stress);
	
	double engr = innerProd(strain, stress);
	engr *= m_vol*0.5;	
	return engr;
	*/
	//new method for energy
	Tensor3x3 F = m_F;
	F.x[0]-=1;
	F.x[4]-=1;
	F.x[8]-=1;
	double engr = innerProd(F, F);
	engr *= m_vol*0.5 * material.getYoung();	
	return engr;	
}


void CShearElement::exportState(const CMeMaterialProperty &material, FILE *fp) const
{
	const double energy = computeElementEnergy(material);
	const double *x = m_F.x;
	fprintf(fp, "Position: %lg %lg %lg\n", m_XI.x, m_XI.y, m_XI.z);
	fprintf(fp, "Force:    %lg %lg %lg\n", m_force.x, m_force.y, m_force.z);
	fprintf(fp, "DefGrad:  %lg %lg %lg\n", x[0], x[1], x[2]);
	fprintf(fp, "          %lg %lg %lg\n", x[3], x[4], x[5]);
	fprintf(fp, "          %lg %lg %lg\n", x[6], x[7], x[8]);
	fprintf(fp, "Total Energy: %lg\n", energy);
}


static int test_func(void)
{
	CMeMaterialProperty mat;
	mat.setPoisson(0.30);
	mat.setDamping(0.2, 0.2);
	//--------------------------------
	const double vol=0.0001;
	Vector3d p0(1,0,0);
	const Vector3d p1(0,0,0);
	CShearElement e(p0, p1, vol);
	//--------------------------------
	const int N=360;
	const double K = 360.0/N;
	const double A1 = (PI*2.0)*K/360.0;
	printf("Angle\t\tForce\t\t\tNorm\tEng\n");
	for (int i=0; i<=N; i++){
		if (i>90) break;
		double angle = i*A1;
		double R=1;
		Vector3d Force;
		double x = R*cos(angle);
		double y = R*sin(angle);
		p0=Vector3d(x, y, 0);
		e.computeNodalForce(p0, p1, mat, Force, NULL);
		const double f2 = Magnitude(Force);
		const double engr = e.computeElementEnergy(mat);
		printf("%lg, %lg, %lg, %lg, %lg, %lg\n", i*K, Force.x, Force.y, Force.z, f2, engr);
		if (i*K>0){
			//Tensor3x3 F = e.getDeformationGradient();
			//printf("%lg, %lg, %lg\n", F.x[0], F.x[1], F.x[2]);
			//printf("%lg, %lg, %lg\n", F.x[3], F.x[4], F.x[5]);
			//printf("%lg, %lg, %lg\n", F.x[6], F.x[7], F.x[8]);
		}
	}
	//exit(0);
	return 1;
}


static int test = test_func();

