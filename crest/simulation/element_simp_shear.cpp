//FILE: element_simp_shear.cpp
#include "element_simp_shear.h"


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


inline void 
ComputeChauchyStrainDeriv(const Vector3d& dg_deriv, Tensor3x3 strain_deriv[3])
{
	const double a = dg_deriv.x;
	const double b = dg_deriv.y;
	const double c = dg_deriv.z;
	const double a2 = a*0.5;
	const double b2 = b*0.5;
	const double c2 = c*0.5;
	double* x = strain_deriv[0].x;
	x[0]=a;
	x[1]=x[3]=b2;
	x[2]=x[6]=c2;
	x[4]=x[5]=x[7]=x[8]=0; 

	x = strain_deriv[1].x;
	x[4]=b;
	x[1]=x[3]=a2;
	x[5]=x[7]=c2;
	x[0]=x[2]=x[6]=x[8]=0;

	x = strain_deriv[2].x;
	x[8]=c;
	x[2]=x[6]=a2;
	x[5]=x[7]=b2;
	x[0]=x[1]=x[3]=x[4]=0;
}


//----------------------------------------------------------
//Compute the ARC deformation gradient F(t_i+1, t_i)
//Input:XI: node I's position at time step t_i
//		TXI: node I's estimiated position at time step	t_i+1
//		XJ: origin (0, 0, 0)
//		TXJ: origin (0, 0, 0)
//		FIJ: incremental deformation gradient - I = pDisp/pX
//----------------------------------------------------------
inline void computeARCDeformationGradient_Stress_Deriv(
	const Vector3d& XI, const Vector3d& TXI, Tensor3x3& FIJ, Tensor3x3& stress, Vector3d& FIJ_deriv)
{
    const Vector3d DIJ =  TXI - XI;
	const double R2_ROD = Magnitude2(XI);
	FIJ_deriv = XI / R2_ROD;
	VectorTensorProduct(DIJ, FIJ_deriv, FIJ);
	stress = FIJ;
	FIJ.AddIdentity();
}


void CSimpShearElement::__ComputeNodalForceFinal2( 
		Tensor3x3& F_total,					//total deformation gradient
		Tensor3x3& F_plastic,				//plastic tensor stored in the elm
		const Vector3d &TXI,				//node i's position at current time step
		const Vector3d &XI,					//node i's position at previous time step
		const double& vol,					//volume of the rod
		const CMeMaterialProperty &material,//material property for element
		Vector3d *fi,						//elastic force for node i, node j's Fj = -Fi;
		Tensor3x3 *stiffness)				//stiffness matrix
{
	if (material.getMaterialType()==1){
		//Chauchy strain tensor method
		(*fi) = m_stiff * (TXI - XI);
		//Need stiffness?
		if (stiffness) 
			(*stiffness) = m_stiff;		
	}
	else{
		//first, DG derivitive matrix, total deformation gradient deriv
		Tensor3x3 F_oldtotal = F_total;
		Vector3d F_arc_deriv;		//arc deformation gradient deriv
		Tensor3x3 strain, straind, stress, stressd;
		computeARCDeformationGradient_Stress_Deriv(XI, TXI, F_total, strain, F_arc_deriv);
		const Vector3d& deriv = F_arc_deriv;
		stress = strain;
		//comp. damping using the total deformation gradient rate vs time dF/dt
		//note that the division of dt^2 is in youngDamp
		const double youngDamp = material.getEffectiveYoungDamp();
		if (youngDamp>0){
			const double3x3 stressdamp = youngDamp * (F_total - F_oldtotal);
			stress += stressdamp;
		}			
		//compute nodal force, then integrate over the element
		(*fi) = DotProd(deriv, stress);
		const double volyoung = -vol * material.getYoung();
		(*fi) *= volyoung;
		if (stiffness){	
			const double k = Magnitude2(deriv)*volyoung*(1.0+youngDamp);
			double* const x = stiffness->x;
			x[0]=x[4]=x[8]=k;
			x[1]=x[2]=x[3]=x[5]=x[6]=x[7]=0; 
		}
	}
}

/*
static double _SCALE_LAMBDA_= 1.0;
static double _SCALE_MIU_   = 1.0;
void setMaterialScaling(const double k_lambda, const double k_miu)
{
	_SCALE_LAMBDA_= k_lambda;
	_SCALE_MIU_   = k_miu;
}
*/
static double _SCALE_YOUNG_= -1.0;
static double _NEW_POISSON_= -1.0;
void setMaterialScaling(const double k_young, const double new_poisson)
{
	_SCALE_YOUNG_ =  k_young;
	_NEW_POISSON_ = new_poisson;
}


//Energey function is (F-I):C:(F-I)
//this method generates better results than the small strain energy
void CSimpShearElement::_computeStiffness( 
		const double& vol,					//volume of the rod
		const CMeMaterialProperty &mtl)		//material property for element
{
	Vector3d F_arc_deriv;	//deformation gradient deriv
	{
		Tensor3x3 F_total, strain;
		computeARCDeformationGradient_Stress_Deriv(m_XI, m_XI, F_total, strain, F_arc_deriv);
	}
	
	//assume isotropic material at this time, compute the stress
	//where stress = elastic stress + viscous stress
	double Lambda, Miu;
	if (_SCALE_YOUNG_==-1.0 || _NEW_POISSON_==-1.0){
		mtl.getLameCofficients(Lambda, Miu);
	}
	else{
		const double young = mtl.getYoung()*_SCALE_YOUNG_;
		YoungsModulus2Lame(young, _NEW_POISSON_, Lambda, Miu);
	}

	Lambda *= -vol; Miu *= -vol;

	//Need stiffness
	Tensor3x3 strain_deriv[3], stress_deriv[3];
	ComputeChauchyStrainDeriv(F_arc_deriv, strain_deriv);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, strain_deriv[0], stress_deriv[0]);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, strain_deriv[1], stress_deriv[1]);
	CMeMaterialProperty::IsotropicMaterialStress(Lambda, Miu, strain_deriv[2], stress_deriv[2]);
	double *x = m_stiff.x;
	x[0] = InnerProd(strain_deriv[0], stress_deriv[0]);
	x[1] = InnerProd(strain_deriv[0], stress_deriv[1]);
	x[2] = InnerProd(strain_deriv[0], stress_deriv[2]);
	x[3] = x[1];
	x[4] = InnerProd(strain_deriv[1], stress_deriv[1]);
	x[5] = InnerProd(strain_deriv[1], stress_deriv[2]);
	x[6] = x[2];
	x[7] = x[5];
	x[8] = InnerProd(strain_deriv[2], stress_deriv[2]);			
}


double CSimpShearElement::computeElementEnergy(const CMeMaterialProperty &material) const
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


void CSimpShearElement::exportState(const CMeMaterialProperty &material, FILE *fp) const
{
	const double energy = computeElementEnergy(material);
	const double *x = m_F.x;
	fprintf(fp, "Position: %lg %lg %lg\n", m_XI.x, m_XI.y, m_XI.z);
	fprintf(fp, "DefGrad:  %lg %lg %lg\n", x[0], x[1], x[2]);
	fprintf(fp, "          %lg %lg %lg\n", x[3], x[4], x[5]);
	fprintf(fp, "          %lg %lg %lg\n", x[6], x[7], x[8]);
	fprintf(fp, "Total Energy: %lg\n", energy);
}


void IsotropicMaterialStressSymbolic(const double3x3& strain, double3x3& stressLambda, double3x3& stressMu)
{
	const double Lambda=1;
	const double Miu=1;
	//copy some strains
	const double *p = strain.x;
	const double e11 = p[0];
	const double e22 = p[4];
	const double e33 = p[8];
	const double M2 = Miu+Miu;
	const double LE123 = Lambda*(e11+e22+e33); 
	const Vector2d m11(LE123, M2*e11); //const double m11= LE123 + M2*e11;
	const Vector2d m22(LE123, M2*e22); //const double m22= LE123 + M2*e22;
	const Vector2d m33(LE123, M2*e33); //const double m33= LE123 + M2*e33;
	//compute stress
	double *qL = stressLambda.x;
	double *qM = stressMu.x;
	/*
	q[0] = m11; 
	q[4] = m22; 
	q[8] = m33;
	q[1] = q[3] = p[1]*M2;
	q[2] = q[6] = p[2]*M2;
	q[5] = q[7] = p[5]*M2;
	*/
	qL[0]=m11.x, qM[0]=m11.y;
	qL[4]=m22.x, qM[4]=m22.y;
	qL[8]=m33.x, qM[8]=m33.y;
	qL[1]=qL[3]=0, qM[1]=qM[3]=p[1]*M2; 
	qL[2]=qL[6]=0, qM[2]=qM[6]=p[2]*M2; 
	qL[5]=qL[7]=0, qM[5]=qM[7]=p[5]*M2;
}


void CSimpShearElement::computeJacobianSymbolic(
	const CMeMaterialProperty* pmtl,
	double3x3& lambdaMat, double3x3& muMat)
{
	//Get deformation gradient deriv
	Vector3d F_arc_deriv;	
	{
		Tensor3x3 F_total, strain;
		computeARCDeformationGradient_Stress_Deriv(m_XI, m_XI, F_total, strain, F_arc_deriv);
	}

	//Strain derivitive
	Tensor3x3 strain_deriv[3], stress_derivL[3], stress_derivM[3];
	ComputeChauchyStrainDeriv(F_arc_deriv, strain_deriv);
	//stress derivitive
	IsotropicMaterialStressSymbolic(strain_deriv[0], stress_derivL[0], stress_derivM[0]);
	IsotropicMaterialStressSymbolic(strain_deriv[1], stress_derivL[1], stress_derivM[1]);
	IsotropicMaterialStressSymbolic(strain_deriv[2], stress_derivL[2], stress_derivM[2]);

	//final jacobian, (Lambda, Miu)
	{
		double3x3 *stress_deriv = stress_derivL;
		double *x = lambdaMat.x;
		x[0] = InnerProd(strain_deriv[0], stress_deriv[0]);
		x[1] = InnerProd(strain_deriv[0], stress_deriv[1]);
		x[2] = InnerProd(strain_deriv[0], stress_deriv[2]);
		x[3] = x[1];
		x[4] = InnerProd(strain_deriv[1], stress_deriv[1]);
		x[5] = InnerProd(strain_deriv[1], stress_deriv[2]);
		x[6] = x[2];
		x[7] = x[5];
		x[8] = InnerProd(strain_deriv[2], stress_deriv[2]);			
	}
	{
		double3x3 *stress_deriv = stress_derivM;
		double *x = muMat.x;
		x[0] = InnerProd(strain_deriv[0], stress_deriv[0]);
		x[1] = InnerProd(strain_deriv[0], stress_deriv[1]);
		x[2] = InnerProd(strain_deriv[0], stress_deriv[2]);
		x[3] = x[1];
		x[4] = InnerProd(strain_deriv[1], stress_deriv[1]);
		x[5] = InnerProd(strain_deriv[1], stress_deriv[2]);
		x[6] = x[2];
		x[7] = x[5];
		x[8] = InnerProd(strain_deriv[2], stress_deriv[2]);			
	}

	lambdaMat*=-m_vol;
	muMat*=-m_vol;

	{//verify the computation
	double lambda, miu, d;
	pmtl->getLameCofficients(lambda, miu);
	double3x3 stiff = lambda*lambdaMat + miu*muMat;
	double3x3 dist = stiff - m_stiff;
	d = InnerProd(dist, dist);
	d+=1e-40;
	}
}
