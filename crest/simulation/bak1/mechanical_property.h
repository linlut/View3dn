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
	double m_young;				//Young's modulus
	double m_poisson;			//Poisson ratio
	double m_young_damp;		//Young's modulus for damping
	double m_poisson_damp;		//Poisson ratio for damping
	//===============================================
	double m_dt;				//time step, can be used to modulate m_lambda_damp & m_miu_damp
	double m_dt_inv2;			//=1.0/(m_dt*m_dt)
	bool m_isIsotropic;

public:
	double m_rho;				//Density
	int m_rigid;				//as rigid as possible? 0: not rigid, 1: rigid

private:
	void computeLameCofficients(void)
	{
		YoungsModulus2Lame(m_young, m_poisson, m_lambda, m_miu);
	}

	void computeLameCofficientsDamping(void)
	{
		YoungsModulus2Lame(m_young_damp*m_young, m_poisson_damp*m_poisson, m_lambda_damp, m_miu_damp);
		const double K = 1.0/m_dt_inv2;	
		m_lambda_damp *= K;
		m_miu_damp *= K;
	}

public:

	bool isIsotropicMaterial(void)
	{
		return m_isIsotropic;
	}

	void getLameCofficients(double &Lambda, double& Miu) const
	{
		Lambda = m_lambda;
		Miu = m_miu;
	}

	double getYoung(void) const
	{
		return m_young;
	}

	double getYoungDamp(void) const
	{
		return m_young_damp;
	}

	void getLameCofficientsDamping(double &Lambda, double& Miu) const
	{
		Lambda = m_lambda_damp;
		Miu = m_miu_damp;
	}
/*
	double getVelocityDamping(void) const
	{
		return m_velocity_damp;
	}
*/
	double getMassDamping(void) const
	{
		return m_mass_damp;
	}

	void getTimeStep(double &dt, double &dt_inv2) const
	{
		dt = m_dt;
		dt_inv2 = m_dt_inv2;
	}

	void setYoung(const double& young)
	{
		m_young = young;
		computeLameCofficients();
	}

	void setPoisson(const double& poisson)
	{
		m_poisson = poisson;
		computeLameCofficients();
	}

	void setDensity(const double &rho)
	{
		m_rho = rho;
	}

	void setDamping(const double& young, const double &poisson)
	{
		m_young_damp = young;
		m_poisson_damp = poisson;
		computeLameCofficientsDamping();
	}
/*
	void setVelocityDamping(const double& ratio)
	{
		m_velocity_damp = ratio;
	}
*/
	void setMassDamping(const double& ratio)
	{
		m_mass_damp = ratio;
	}

	void setTimeStepLength(const double& t)
	{
		m_dt = 1.0; //t;
		m_dt_inv2 = 1.0/(m_dt*m_dt);
		computeLameCofficientsDamping();
	}

	void init(void)
	{
		computeLameCofficients();
		computeLameCofficientsDamping();
	}

	//default constructor use the international units meter, kg
	CMeMaterialProperty(void)
	{
		m_young = 1E6;
		m_poisson = 0.30;
		m_young_damp = 0.01;
		m_poisson_damp = 0.01;	//m_poisson;
		m_rho = 1000.0;
		m_dt =1.0;				//time step, can be used to modulate m_lambda_damp & m_miu_damp
		m_dt_inv2 = 1.0;
		m_velocity_damp = 5.00e-4;
		m_mass_damp = 0;
		
		m_rigid = 0;
		m_isIsotropic = true;
		init();
	}

	static void IsotropicMaterialStress(const double& Lambda, const double& Miu, const double3x3& strain, double3x3& stress)
	{
		//copy some strains
		const double *p = strain.x;
		const double& e11 = p[0];
		const double& e22 = p[4];
		const double& e33 = p[8];
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

};



//
class CVertexConstraints
{
private:
	unsigned int m_nTag;

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