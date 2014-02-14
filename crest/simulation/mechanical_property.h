//FILE: mechanical_property.h

#ifndef _INC_MECHANICAL_PROP_H_
#define _INC_MECHANICAL_PROP_H_

#include <matrix3x3.h>

//Convert young's modulus to lame cofficients
inline void YoungsModulus2Lame(const double& YOUNG, const double& POISS, double &Lambda, double &Miu)
{
	const double POISS1 = 1.00/(1.0+POISS);
	//lambda = vE/(1+v)/(1-2v)
	Lambda = YOUNG * POISS *POISS1 /(1.0- POISS - POISS);
	//miu = E/2/(1+v)
	Miu = YOUNG * POISS1 * 0.5;
}


class CMeMaterialProperty
{
private:
	double m_lambda;		
	double m_miu;
	double m_lambda_damp;
	double m_miu_damp;
	double m_velocity_damp;
	double m_mass_damp;

	//===============================================
	double m_rho;				//Density
	double m_young;				//Young's modulus
	double m_poisson;			//Poisson ratio
	double m_young_damp;		//Young's modulus for damping
	double m_effective_young_damp;	// m_young_damp/dt/dt*1e6
	double m_poisson_damp;		//Poisson ratio for damping
	double m_gamma1;			//gamma1 for plasitc deformation
	double m_gamma2;			//gamma1 for plasitc deformation
	//===============================================
	double m_dt;				//time step, can be used to modulate m_lambda_damp & m_miu_damp
	int m_nMaterialType;		//Type 0: very simple, only use Young's modulus
								//Type 1: Isotropic, with Young's modulus and poisson
								//Type 2: tba
	
public:
	int m_rigid;				//as rigid as possible? 0: not rigid, 1: rigid

private:
	void computeLameCofficients(void)
	{
		YoungsModulus2Lame(m_young, m_poisson, m_lambda, m_miu);
	}

	void computeLameCofficientsDamping(void)
	{
		YoungsModulus2Lame(m_young_damp*m_young, m_poisson_damp*m_poisson, m_lambda_damp, m_miu_damp);
		const double K = 1.0/m_dt/m_dt;	
		m_lambda_damp *= K;
		m_miu_damp *= K;
	}

public:
	int getMaterialType(void) const
	{
		return m_nMaterialType;
	}

	void setMaterialType(const int t)
	{
		m_nMaterialType = t;
	}

	bool isIsotropicMaterial(void) const
	{
		return (m_nMaterialType == 1);
	}

	void getLameCofficients(double &Lambda, double& Miu) const
	{
		Lambda = m_lambda;
		Miu = m_miu;
	}

	inline double getDensity(void) const
	{
		return m_rho;
	}

	inline double getYoung(void) const
	{
		return m_young;
	}

	inline double getPoisson(void) const
	{
		return m_poisson;
	}

	inline double getYoungDamp(void) const
	{
		return m_young_damp;
	}

	inline void getLameCofficientsDamping(double &Lambda, double& Miu) const
	{
		Lambda = m_lambda_damp;
		Miu = m_miu_damp;
	}

	inline double getVelocityDamping(void) const
	{
		return m_velocity_damp;
	}

	inline double getMassDamping(void) const
	{
		return m_mass_damp;
	}

	inline double getTimeStep(void) const
	{
		return m_dt;
	}

	inline double getEffectiveYoungDamp(void) const 
	{
		return m_effective_young_damp;
	}

	inline void setYoung(const double& young)
	{
		m_young = young;
		computeLameCofficients();
	}

	inline void setPoisson(const double& poisson)
	{
		m_poisson = poisson;
		computeLameCofficients();
	}

	inline void setDensity(const double &rho)
	{
		m_rho = rho;
	}

	inline void setDamping(const double& young, const double &poisson)
	{
		m_young_damp = young;
		m_poisson_damp = poisson;
		computeLameCofficientsDamping();
	}

	inline void setVelocityDamping(const double& ratio)
	{
		m_velocity_damp = ratio;
	}

	inline void setMassDamping(const double& ratio)
	{
		m_mass_damp = ratio;
	}

	inline void setTimeStepLength(const double& t)
	{
		m_dt = t;
		computeLameCofficientsDamping();
	}

	inline void setGamma1(const double g)
	{
		m_gamma1 = g;
	}

	inline void setGamma2(const double g)
	{
		m_gamma2 = g;
	}

	inline double getGamma1(void) const
	{
		return m_gamma1;
	}

	inline double getGamma2(void) const
	{
		return m_gamma2;
	}

	inline void init(void)
	{
		computeLameCofficients();
		computeLameCofficientsDamping();
	}

	inline void computeEffectiveYoungDamp(void)
	{
		const double dt2 = m_dt * m_dt;
		m_effective_young_damp = m_young_damp /dt2 * 1e-6;
	}

	//default constructor use the international units meter, kg
	CMeMaterialProperty(void)
	{
		m_young = 1.0E+6;
		m_poisson = 0.30;
		m_young_damp = 0.00;
		m_effective_young_damp = 0.00;
		m_poisson_damp = 0.00;		//m_poisson;
		m_rho = 1000.0;
		m_dt = 1e-3;				//time step, can be used to modulate m_lambda_damp & m_miu_damp
		m_velocity_damp = 0.00e-4;
		m_mass_damp = 5e-4;
		m_gamma1 = m_gamma2 = 0;
		
		m_rigid = 0;
		m_nMaterialType = 0;		//very simple material, only use Young's modulus
		init();
	}

	static void IsotropicMaterialStress(const double& Lambda, const double& Miu, const double3x3& strain, double3x3& stress)
	{
		//copy some strains
		const double *p = strain.x;
		const double e11 = p[0];
		const double e22 = p[4];
		const double e33 = p[8];
		const double M2 = Miu+Miu;
		const double LE123 = Lambda*(e11+e22+e33); 
		const double m11= LE123 + M2*e11;
		const double m22= LE123 + M2*e22;
		const double m33= LE123 + M2*e33;
		//compute stress
		double *q = stress.x;
		q[0] = m11; q[4] = m22; q[8] = m33;
		q[1] = q[3] = p[1]*M2;
		q[2] = q[6] = p[2]*M2;
		q[5] = q[7] = p[5]*M2;
	}

	static void IsotropicMaterialNSStress(const double& Lambda, const double& Miu, const double3x3& strain, double3x3& stress)
	{
		//copy some strains
		const double *p = strain.x;
		const double e11 = p[0];
		const double e22 = p[4];
		const double e33 = p[8];
		const double M2 = (Miu+Miu);
		const double LE123 = Lambda*(e11+e22+e33); 
		const double m11= LE123 + M2*e11;
		const double m22= LE123 + M2*e22;
		const double m33= LE123 + M2*e33;
		//compute stress
		double *q = stress.x;
		q[0] = m11; q[4] = m22; q[8] = m33;
		q[1] = p[1]*M2;
		q[3] = p[3]*M2;
		q[2] = p[2]*M2;
		q[6] = p[6]*M2;
		q[5] = p[5]*M2;
		q[7] = p[7]*M2;
	}

	inline void isotropicMaterialStress(const double3x3& strain, double3x3& stress) const
	{
		IsotropicMaterialStress(m_lambda, m_miu, strain, stress);
	}

	inline void isotropicMaterialNSStress(const double3x3& strain, double3x3& stress) const
	{
		IsotropicMaterialNSStress(m_lambda, m_miu, strain, stress);
	}
};

typedef CMeMaterialProperty* CMeMaterialPropertyPtr;

//
class CVertexConstraints
{
private:
	unsigned int m_nTag;
	unsigned int _tmp;

public:
	CVertexConstraints(void)
	{
		m_nTag  = 0;
	}

	bool hasNoConstraints(void) const
	{
		return (m_nTag==0);
	}

	bool hasFixedPositionXYZ(void) const
	{
		const unsigned int BITS = 0x00000007;
		return ((m_nTag & BITS)==BITS);
	}

	void setFixedPositionXYZ(void)
	{
		const unsigned int BITS = 0x00000007;
		m_nTag |= BITS;
	}
};


#endif