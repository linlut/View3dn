//FILE: element_shear.cpp
#include "element_shear.h"

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

static inline double SymetricTensorDotProd(const Tensor3x3& A, const Tensor3x3& B)
{
	const double *p = A.x;
	const double *q = B.x;
	const double r1 = p[0]*q[0]+p[4]*q[4]+p[8]*q[8];
	const double r2 = p[1]*q[1]+p[2]*q[2]+p[5]*q[5];
	return r1+r2+r2;
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


//Compute the total deformation gradient F 
inline void ComputeDeformationGradient(
	const Vector3d &XI, const Vector3d &TXI, const Tensor3x3 &F_oldtotal, 
	Tensor3x3 &F_total, Vector3d &arc_deriv)
{
	Tensor3x3 F_arc;
	//comp. incremental deformation gradient
	computeARCDeformationGradientAndDeriv(XI, TXI, F_arc, arc_deriv);
	//comp. the total deformation gradient
	F_total = F_arc * F_oldtotal;
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

/*
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
	ComputeDeformationGradient(m_XI, TXI, m_F, F_total, F_arc_deriv);
	//total strain tensor and stress
	ASSERT0(fi!=NULL);
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
		F_total_rate = F_total - m_F;
		ChauchyStrainRate(F_total_rate, straind);
		LambdaDamp*=vol1; MiuDamp*=vol1;
		CMeMaterialProperty::IsotropicMaterialStress(LambdaDamp, MiuDamp, straind, stressd);
		stress+=stressd;
	}
	//Compute DG derivitive matrix, then the final force on node XI
	deriv = F_arc_deriv * m_F;
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

	//update the stored var.
	_updateState(TXI, F_total);
}
*/

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
		Tensor3x3 *stiffness)				//stiffness matrix
{
	//first, DG derivitive matrix, total deformation gradient deriv
	Vector3d F_arc_deriv;					//arc deformation gradient deriv
	ComputeDeformationGradient(XI, TXI, F_oldtotal, F_total, F_arc_deriv);
	//compute simple stress
	Tensor3x3 stress = F_total; stress.MinusIdentity();
	//simplified alg. to compute F_arc_deriv * m_F, since F_arc_deriv has redundency
	const Vector3d deriv = F_arc_deriv * F_total;  
	//comp. damping using the total deformation gradient rate vs time dF/dt
	//note that the division of dt^2 is in youngDamp
	const double youngDamp = material.getEffectiveYoungDamp();
	const double3x3 stressdamp = youngDamp * (F_total - F_oldtotal_damp);
	stress += stressdamp;
	switch (material.getMaterialType()){
	default:
	case 0: 
		{//compute nodal force, then integrate over the element
		(*fi) = DotProd(deriv, stress);
		const double volyoung = -vol * material.getYoung();
		(*fi) *= volyoung;
		if (stiffness){	
			const double k = Magnitude2(deriv)*volyoung*(1.0+youngDamp);
			double* const x = stiffness->x;
			x[0]=x[4]=x[8]=k;
			//x[1]=x[2]=x[3]=x[5]=x[6]=x[7]=0; 
		}
		}
		break;
	case 1: 
		{//use Young and Poisson
		material.isotropicMaterialNSStress(stress, stress);
		(*fi) = DotProd(deriv, stress)*(-vol);
		if (stiffness){	
			double3x3 strainderiv, stressderiv;
			Vector3d *row = (Vector3d*)strainderiv.x;
			row[0]=row[1]=row[2]= deriv;
			material.isotropicMaterialNSStress(strainderiv, stressderiv);
			Vector3d stiffdiagonal = DotProd(deriv, stressderiv)*(-vol);
			double *x = stiffness->x;
			x[0]=stiffdiagonal.x, x[4]=stiffdiagonal.y, x[8]=stiffdiagonal.z;
			x[1]=x[2]=x[3]=x[5]=x[6]=x[7]=0; 
		}
		}
		break;
	case 2: 
		{//material type 0 + plastic rod
		stress -= stressdamp;
		const double3x3 ee = stress - F_plastic;
		const double lambda = 0.3333333333* ee.Trace();
		double3x3 e1=ee; e1.x[0]-=lambda, e1.x[4]-=lambda, e1.x[8]-=lambda;
		const double n1 = e1.FrobeniusNorm();
		const double normdif = n1 - material.getGamma1();
		if (normdif<=0){
			//elastic strain only
			stress = ee;
		}
		else{	
			//enter the plastic region
			const double3x3 delta_ep = (normdif/(n1+1e-20))*e1;
			F_plastic += delta_ep;
			const double n2 = F_plastic.FrobeniusNorm();
			const double K = _MIN_(1.0, (material.getGamma2()/n2));
			if (K!=1.0) F_plastic *= K;
			stress -= F_plastic;
		}
		stress += stressdamp;
		(*fi) = DotProd(deriv, stress);
		const double volyoung = -vol * material.getYoung();
		(*fi) *= volyoung;
		if (stiffness){	
			assert(0);
		}
		}
		break;
	}
}

double CShearElement::computeElementEnergy(const CMeMaterialProperty &material) const
{
	double engr = 0;
	const int mattype = material.getMaterialType();

	switch (mattype){
	case 0:  //material type where only Young's modulus is used
		{
		Tensor3x3 F = m_F;
		F.MinusIdentity();
		engr = InnerProd(F, F);
		engr *= m_vol*0.5 * material.getYoung();	
		}
		break;
	case 1:	//isotropic material
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
		break;
	default:
		break;
	}

	return engr;	
}


void CShearElement::exportState(const CMeMaterialProperty &material, FILE *fp) const
{
	const double energy = computeElementEnergy(material);
	const double *x = m_F.x;
	fprintf(fp, "Position: %lg %lg %lg\n", m_XI.x, m_XI.y, m_XI.z);
	fprintf(fp, "DefGrad:  %lg %lg %lg\n", x[0], x[1], x[2]);
	fprintf(fp, "          %lg %lg %lg\n", x[3], x[4], x[5]);
	fprintf(fp, "          %lg %lg %lg\n", x[6], x[7], x[8]);
	fprintf(fp, "Total Energy: %lg\n", energy);
}

/*
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
	double3x3 stiffness;

	printf("Angle\t\tForce\t\t\tNorm\tEng\n");
	for (int i=0; i<=N; i++){
		if (i>90) break;
		double angle = i*A1;
		double R=1;
		Vector3d Force;
		double x = R*cos(angle);
		double y = R*sin(angle);
		p0=Vector3d(x, y, 0);
		e.computeNodalForce(p0, p1, mat, Force, &stiffness);
		const double f2 = Magnitude(Force);
		const double engr = e.computeElementEnergy(mat);
		printf("%lg, %lg, %lg, %lg, %lg, %lg, %lg\n", i*K, Force.x, Force.y, Force.z, f2, engr, stiffness.x[0]);
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
*/
