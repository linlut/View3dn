//FILE: time_integration.h
#ifndef _INC_TIMEINTEGRATION_ZN200808_H_
#define _INC_TIMEINTEGRATION_ZN200808_H_

#include <sysconf.h>
#include <assert.h>
#include <vector>
#include <vectorall.h>


class CSimuEntity;
class CSparseMatrix33;


class ODEIntegrator
{
protected:
	bool m_bIsImplicit;
	unsigned char m_cPreconditionerType;//in implicit methods, there must be a preconditioner
										//0: the diagonal preconditoner
										//1: incomplete-cholesky preconditioner
	bool __pad[2];

	int m_nv;							//# of vertices
	double m_avgmass;
	CSparseMatrix33 *m_pSitffnessMat;	//the sparse stiffness matrix
	CSimuEntity *m_pObj;
	int m_nTotalIterationCount;			//iteration number in simulation

protected:
	static std::vector<double> m_vbuffer;

public:
	ODEIntegrator(CSimuEntity *pobj);

	virtual ~ODEIntegrator(void);

	virtual double *getMassBuffer(void)
	{
		return NULL;
	}

	inline bool isImplicit(void)
	{
		return m_bIsImplicit;
	}

	inline CSparseMatrix33 *getSparseMatrix(void)
	{
		return m_pSitffnessMat;
	}

	inline double *getVertexArrayBuffer()
	{
		return &m_vbuffer[0];
	}

	inline double getAverageMass(void)
	{
		return m_avgmass;
	}

	virtual void doIntegration(const unsigned int timeid, const double& dt)=NULL;

	virtual void resize(const int nv)=NULL;

	virtual void fixIssuesForFixedPositionConstraint(const int *vid, const int nv){};

};


class VerletIntegrator : public ODEIntegrator
{
private:
	vector<Vector3d> m_vertexPos0;

public:
	VerletIntegrator(CSimuEntity *pobj);
	virtual ~VerletIntegrator(void);
	virtual void doIntegration(const unsigned int timeid, const double& dt);
	virtual void resize(const int nv);
	virtual void fixIssuesForFixedPositionConstraint(const int *vid, const int nv);
};


class CICPrecondMatrix;
class ImplicitEulerIntegrator : public ODEIntegrator
{
private:
	vector<double> m_mass;	//this mass is formed with constraint, 
							//which may be different from the real mass, e.g. +inf for fixed nodes
protected:
	CICPrecondMatrix *m_pPrecondMatrix;

public:
	ImplicitEulerIntegrator(CSimuEntity *pobj, const double *pmass, const int stridemass);
	virtual ~ImplicitEulerIntegrator(void);
	
	virtual void doIntegration(const unsigned int timeid, const double& dt);
	virtual void resize(const int nv);

	inline double *getMassBuffer(void)
	{
		double *p=NULL;
		if (m_mass.size()) p = &m_mass[0];
		return p;
	}
};


class ImplicitTrapezoidalIntegrator : public ImplicitEulerIntegrator
{
public:
	ImplicitTrapezoidalIntegrator(CSimuEntity *pobj, const double *pmass, const int stridemass):
		ImplicitEulerIntegrator(pobj, pmass, stridemass)
		{}
	virtual ~ImplicitTrapezoidalIntegrator(void)
		{}

	virtual void doIntegration(const unsigned int timeid, const double& dt);
};


class ExplicitStableIntegrator : public ImplicitEulerIntegrator
{
public:
	ExplicitStableIntegrator(CSimuEntity *pobj, const double *pmass, const int stridemass):
		ImplicitEulerIntegrator(pobj, pmass, stridemass)
		{}
	virtual ~ExplicitStableIntegrator(void)
		{}

	virtual void doIntegration(const unsigned int timeid, const double& dt);
};


class BDF2Integrator : public ODEIntegrator
{
private:
	vector<double> m_mass;	//this mass is formed with constraint, 
							//which may be different from the real mass, e.g. +inf for fixed nodes
	CICPrecondMatrix *m_pPrecondMatrix;

public:
	BDF2Integrator(CSimuEntity *pobj, const double *pmass, const int stridemass);
	virtual ~BDF2Integrator(void);
	
	virtual void doIntegration(const unsigned int timeid, const double& dt);
	virtual void resize(const int nv);

	inline double *getMassBuffer(void)
	{
		double *p=NULL;
		if (m_mass.size()) p = &m_mass[0];
		return p;
	}

};


class NewmarkIntegrator : public ODEIntegrator
{
private:
	vector<double> m_mass;		//this mass is formed with constraint, 
	CICPrecondMatrix *m_pPrecondMatrix;

public:
	NewmarkIntegrator(CSimuEntity *pobj, const double *pmass, const int stridemass);
	virtual ~NewmarkIntegrator(void);
	virtual void doIntegration(const unsigned int timeid, const double& dt);
	virtual void resize(const int nv);

	inline double *getMassBuffer(void)
	{
		double *p=NULL;
		if (m_mass.size()) p = &m_mass[0];
		return p;
	}
};


class ImplicitGMPIntegrator : public ImplicitEulerIntegrator
{
public:
	ImplicitGMPIntegrator(CSimuEntity *pobj, const double *pmass, const int stridemass):
		ImplicitEulerIntegrator(pobj, pmass, stridemass)
		{}
	virtual ~ImplicitGMPIntegrator(void)
		{}

	virtual void doIntegration(const unsigned int timeid, const double& dt);
};


#endif
