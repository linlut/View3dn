//FILE: time_integration.cpp

#include "time_integration.h"
#include "simu_entity.h"
#include "sparseMatrix.h"
#include "zsparse_solver.h"
#include "zicprecond_matrix.h"


ODEIntegrator::ODEIntegrator(CSimuEntity *pobj)
{
	m_nv = 0;
	m_avgmass = 0;
	m_bIsImplicit = false;
	m_pSitffnessMat = NULL;
	m_pObj = pobj;
	assert(pobj!=NULL);
	m_nTotalIterationCount = 0;
}


ODEIntegrator::~ODEIntegrator(void)
{
	if (m_pSitffnessMat)
		delete m_pSitffnessMat, m_pSitffnessMat = NULL;
	//printf("Total iteration count is %d.\n", m_nTotalIterationCount);
}


/*
void VelocityVerletIntegration(CSimuEntity &obj, const unsigned int timeid, const double& dt)
{
	int i;
	const double dt05 = 0.5*dt;
	const double dt05dt = dt05*dt;
	const int nv = obj.getVertexCount();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//Advect the location
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d &x = node->m_pos;
		Vector3d &x0 = node->m_tmp0;
		Vector3d &v = node->m_velocity;
		Vector3d &a = node->m_acc;
		Vector3d &a0 = node->m_tmp2;
		x0 = x;
		a0 = a;
		x += v*dt + a*dt05dt;		//update x	
		//v += a*dt05;				//update v to half way
	}

	//apply position constraint filters, note that time is in milisec.
	const double current_tm = 1000.0* dt * timeid;
	const double dt1000 = dt*1000.0;
	obj.performEdgeRelaxation(current_tm, dt1000);

	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid, false);

	//update velocity
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d &v = node->m_velocity;
		Vector3d &a = node->m_acc;
		Vector3d &a0 = node->m_tmp2;
		v += (a+a0)*dt05;				//update v 
	}
}


void VelocityBasedIntegration(CSimuEntity &obj, const unsigned int timeid, const double& dt)
{
	int i;
	const double dt05 = 0.5*dt;
	const double dt05dt = dt05*dt;
	const int nv = obj.getVertexCount();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//Advect the location
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d &x = node->m_pos;
		Vector3d &v = node->m_velocity;
		Vector3d &x0 = node->m_tmp0;
		Vector3d &v0 = node->m_tmp1;
		Vector3d &a = node->m_acc;
		x0 = x;
		v0 = v;
		x = x0 + dt05*v0;		//update x	
		v = v0 + a*dt05;		//update v(t+1/2)
	}

	//apply position constraint filters, note that time is in milisec.
	const double current_tm = 1000.0* dt * timeid;
	const double dt1000 = dt*1000.0;
	obj.performEdgeRelaxation(current_tm, dt1000);

	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid);

	//update velocity
	const double factor = 0.1;
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d &x = node->m_pos;
		Vector3d &v = node->m_velocity;
		Vector3d &x0 = node->m_tmp0;
		Vector3d &v0 = node->m_tmp1;
		Vector3d &a = node->m_acc;
		v += dt05*a;			//update v using rectangular integration
		x = x0+dt05*(v+v0);
	}

	//update state vars
	//obj.updateState();
}
*/

vector<double> ODEIntegrator::m_vbuffer;

VerletIntegrator::VerletIntegrator(CSimuEntity *pobj): 
	ODEIntegrator(pobj)
{
	const int nv = pobj->getVertexCount();
	resize(nv);
}

VerletIntegrator::~VerletIntegrator(void)
{}

void VerletIntegrator::resize(const int nv)
{
	if (m_vertexPos0.size()<nv) m_vertexPos0.resize(nv);
	m_nv = nv;
}

void VerletIntegrator::fixIssuesForFixedPositionConstraint(const int *vbuff, const int nv)
{
	CSimuEntity::VertexInfo *pvert = m_pObj->getVertexInfoPtr();
	for (int i=0; i<nv; i++){
		const int vid = vbuff[i];
		Vector3d & x0 = pvert[vid].m_pos0;
		Vector3d & x = pvert[vid].m_pos;
		m_vertexPos0[vid] = x;
	}
}


void VerletIntegrator::doIntegration(const unsigned int timeid, const double& dt)
{
	CSimuEntity &obj = *m_pObj;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	double dtdt = dt*dt;

	const CMeMaterialProperty* pmat = obj.getMaterial();
	const double vdamp_ratio = pmat->getMassDamping();
	const double vdamp2  = 2.0 - vdamp_ratio;
	const double vdamp1  = 1.0 - vdamp_ratio;
	dtdt*=vdamp1;

	//Advect the location
	Vector3d *q = &m_vertexPos0[0];
	if (timeid != 0){
		for (int i=0; i<nv; i++, q++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = q[0];			
			Vector3d x2 = x;			
			Vector3d &a = node->m_acc;
			x = vdamp2*x2 - vdamp1*x1 + dtdt*a;	//update x
			x1 = x2;
		}
	}
	else{
		//the initial time step =0 is advanced in a simple way
		//since at the very begining we have difficulty to use the above equation
		//therefore, we just approximate the first step
		const double dtdt05 = 0.5*dt*dt;
		for (int i=0; i<nv; i++, q++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = q[0];			
			Vector3d &v = node->m_velocity;			
			Vector3d &a = node->m_acc;
			x1 = x;								//backup pos in t
			x += v*dt + a*dtdt05;				//update x
		}
	}

	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);

	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid);
}



//====ImplicitEulerIntegrator=============================
ImplicitEulerIntegrator::ImplicitEulerIntegrator(
	CSimuEntity *pobj, const double *_pmass, const int stridemass):
	ODEIntegrator(pobj)
{
	m_bIsImplicit = true;
	const int nv = pobj->getVertexCount();
	this->resize(nv);

	//IC preconditioner 
	m_pPrecondMatrix = NULL;

	const double *pmass = _pmass;
	double totalmass = 0;
	for (int i=0; i<nv; i++, pmass+=stridemass){
		m_mass[i] = *pmass;
		totalmass += *pmass;
	}
	m_avgmass = totalmass/nv;
	//printf("Vertex num %d, avg mass %lg.\n", nv, m_avgmass);
}

ImplicitEulerIntegrator::~ImplicitEulerIntegrator(void)
{
	SafeDelete(m_pPrecondMatrix);
}

void ImplicitEulerIntegrator::resize(const int nv)
{
	const unsigned int nfloat = 9*nv;
	if (m_vbuffer.size()<nfloat) m_vbuffer.resize(nfloat);
	if (m_mass.size()<nv) m_mass.resize(nv);

	const int stiffnesstype = m_pObj->isStiffnessMatrix3X3() ? 1 : 0;
	if (m_pSitffnessMat==NULL || m_pSitffnessMat->size()!=nv){
		delete m_pSitffnessMat;
		m_pSitffnessMat = new CSparseMatrix33(nv, stiffnesstype);
	}
	else{
		m_pSitffnessMat->init(nv);
	}
	m_nv = nv;
}

void ImplicitEulerIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double ninvdt = -1.0/dt;
	const double ninvdtdt = ninvdt/dt;
	const double dt05 = 0.50*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	const int nv = obj.getVertexCount();	
	const bool conststiff = obj.hasConstantStiffness();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	if (timeid==0){
		const double dt05dt = dt05*dt;
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;		//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			x += dt*v + a*dt05dt;			//advance x one step using the method in velocity verlet
		}
		goto COMPUTE_ACC;
	}

	//Solve eqn AX = B, first compute right term B
	Vector3d* const b = (Vector3d*)this->getVertexArrayBuffer();
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	A.multiplyVector(&pnodearray[0].m_velocity, sizeof(CSimuEntity::VertexInfo), b);
	for (int i=0; i<nv; i++) b[i] = pnodearray[i].m_force*ninvdt - b[i]; 

	//Compute pF/pX - M/h^2, prepare left term A
	const double* pmass = this->getMassBuffer();
	if (timeid==1 || (!conststiff)) A.addDiagonalMatrix(pmass, ninvdtdt);

	//Now find deltaV, where the initial guess is zero (set in CG alg.)
	const double eps=1.0e-5; const int maxiter=30;
	Vector3d* const dV = b+nv;
	int nitr;
	if (conststiff){
		if (m_pPrecondMatrix==NULL){ //Incomplete Cholesky preconditioner
			A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, dV, eps, maxiter);
	}else{
		Vector3d* const C = dV + nv; //Diagonal preconditioner
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, dV, eps, maxiter);
		A.clear();					 //Set elements to 0 for next time step
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaV
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		node->m_velocity += dV[i]; 
		node->m_pos += dt*node->m_velocity; 
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}


//====Implicit Trapezoidal Integrator=============================
void ImplicitTrapezoidalIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double ninvdt = -1.0/dt;
	const double ninvdt4 = 4.0*ninvdt;
	const double invdtdt = -ninvdt/dt;
	const double invdtdt8 = 8.0*invdtdt;
	const double dt05 = 0.50*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	const bool conststiff = obj.hasConstantStiffness();
	const int nv = obj.getVertexCount();	
	
	//initial step;
	if (timeid==0){
		const double dt05dt = dt05*dt;
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			v+= dt*a, x += v*dt + a*dt05dt;		//advance x one step using the velocity verlet
		}
		goto COMPUTE_ACC;
	}

	//Solve eqn AX = B, first compute left term A
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	const double* pmass = this->getMassBuffer();
	if ((!conststiff) || (timeid==1)) A.addDiagonalMatrix(pmass, -4.0*invdtdt);
	//then the right part of eq, which is b
	Vector3d* const b = (Vector3d*)this->getVertexArrayBuffer();
	A.multiplyVector(&pnodearray[0].m_velocity, sizeof(CSimuEntity::VertexInfo), b);
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *n = &pnodearray[i];
		b[i] = n->m_force*ninvdt4 - 2.0*b[i] - n->m_velocity*(pmass[i]*invdtdt8); 
	}

	//Now find deltaV, where the initial guess is zero (set in CG alg.)
	int nitr;
	const double eps=1.0e-5; const int maxiter=30;
	Vector3d* const dV = b+nv;
	if (conststiff){//Incomplete Cholesky preconditioner
		if (m_pPrecondMatrix==NULL){
			A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, dV, eps, maxiter);
	}else{//Build diagonal preconditioner for nonconstant stiffness
		Vector3d* const C = dV + nv;		
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, dV, eps, maxiter);
		A.clear();
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaV
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d& v0 = node->m_velocity; 
		const Vector3d v = v0 + dV[i]; 
		node->m_pos += dt05*(v + v0), v0 = v; 
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}


void ExplicitStableIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double invdt = 1.0/dt;
	const double invdt4 = 4.0*invdt;
	const double invdtdt = invdt*invdt;
	const double dt05 = 0.50*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	const bool conststiff = obj.hasConstantStiffness();
	const int nv = obj.getVertexCount();	
	
	//initial step;
	if (timeid==0){
		const double dt05dt = dt05*dt;
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			node->m_velocity0 = a;
			v+= dt*a, x += v*dt + a*dt05dt;		//advance x one step using the velocity verlet
		}
		goto COMPUTE_ACC;
	}

	//Solve eqn AX = B, first compute left term A
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	const double* pmass = this->getMassBuffer();
	if (timeid==1) A.addDiagonalMatrix(pmass, 4.0*invdtdt);
	//then the right part of eq, which is b
	Vector3d* const b = (Vector3d*)this->getVertexArrayBuffer();
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *n = &pnodearray[i];
		b[i] = (n->m_velocity*invdt4 + 2.0*n->m_acc)*pmass[i]; 
	}

	//Now find deltaV, where the initial guess is zero (set in CG alg.)
	int nitr;
	const double eps=1.0e-5; const int maxiter=25;
	Vector3d* const dX = b+nv;
	if (conststiff){//Incomplete Cholesky preconditioner
		if (m_pPrecondMatrix==NULL){
			A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, dX, eps, maxiter);
	}else{//Build diagonal preconditioner for nonconstant stiffness
		Vector3d* const C = dX + nv;		
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, dX, eps, maxiter);
		A.clear();
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaX
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		node->m_pos += dX[i]; 
		node->m_velocity0 = node->m_acc;
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, false);
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		node->m_velocity += dt05*(node->m_acc + node->m_velocity0); 
	}
}



//====Implicit BDF2 Integrator=============================
BDF2Integrator::BDF2Integrator(
	CSimuEntity *pobj, const double *_pmass, const int stridemass):
	ODEIntegrator(pobj)
{
	m_bIsImplicit = true;
	const int nv = pobj->getVertexCount();
	this->resize(nv);

	//IC preconditioner 
	m_pPrecondMatrix = NULL;

	const double *pmass = _pmass;
	double totalmass = 0;
	for (int i=0; i<nv; i++, pmass+=stridemass){
		m_mass[i] = *pmass;
		totalmass += *pmass;
	}
	m_avgmass = totalmass/nv;
	printf("Vertex num %d, avg mass %lg.\n", nv, m_avgmass);
}

BDF2Integrator::~BDF2Integrator(void)
{
	SafeDelete(m_pPrecondMatrix);
}

void BDF2Integrator::resize(const int nv)
{
	const unsigned int nfloat = 9*nv;
	if (m_vbuffer.size()<nfloat) m_vbuffer.resize(nfloat);

	if (m_mass.size()<nv) m_mass.resize(nv);

	//allocate buffer
	const int stiffnesstype = m_pObj->isStiffnessMatrix3X3()?1:0;
	if (m_pSitffnessMat==NULL)
		m_pSitffnessMat = new CSparseMatrix33(nv, stiffnesstype);
	else if (m_pSitffnessMat->size()!=nv){
		delete m_pSitffnessMat;
		m_pSitffnessMat = new CSparseMatrix33(nv, stiffnesstype);
	}
	else
		m_pSitffnessMat->init(nv);
	ASSERT0(m_pSitffnessMat!=NULL);

	//copy
	m_nv = nv;
}


void BDF2Integrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double dt05 = 0.50*dt;
	const double invdt = 1.0/dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	const int nv = obj.getVertexCount();	
	const bool conststiff = obj.hasConstantStiffness();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	if (timeid==0){
		const double dt05dt = dt05*dt;
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;		//give alias names
			Vector3d &x0 = node->m_pos0;	//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &v0 = node->m_velocity0;
			Vector3d &a = node->m_acc;
			v0 = v; v += a*dt;
			x0 = x; x += dt*v0 + a*dt05dt;	
		}
		goto COMPUTE_ACC;
	}

	//Solve eqn Ax = b, first prepare A
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	const double* pmass = (const double*)this->getMassBuffer();
	if ((!conststiff) || (timeid==1)) 
		A.addDiagonalMatrix(pmass, -2.250*invdt*invdt);
	//then prepare vector b
	Vector3d* x = (Vector3d*)this->getVertexArrayBuffer();
	Vector3d* b = x + nv;
	const double k1 = -0.75*invdt*invdt;
	const double k2 = -0.5*invdt;
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo* n = &pnodearray[i];
		const Vector3d dx = (pmass[i]*k1)*(n->m_pos - n->m_pos0);
		const Vector3d dv = (pmass[i]*k2)*(4.0*n->m_velocity - n->m_velocity0);
		b[i] = dx + dv - n->m_force;
	}

	//solve the EQ, find deltaV, where initial guess of deltaV is zero, set in CG alg.
	const double eps=1.0e-5; const int maxiter=25;
	int nitr;
	if (conststiff && m_pPrecondMatrix==NULL){ //Incomplete Cholesky preconditioner
		A.deleteZeroElements();
		m_pPrecondMatrix = new CICPrecondMatrix(A);
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, x, eps, maxiter);
	}
	else{
		Vector3d* const C = b + nv;				//build diagonal preconditioner
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, x, eps, maxiter);
		A.clear();
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using new position x^(n+1)
	const double factor2 = 1.5*invdt;
	const double factor1 = -2.0*invdt;
	const double factor0 = 0.5*invdt;
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		Vector3d &x1 = node->m_pos, &x0 = node->m_pos0;
		const Vector3d x2 = x[i]+x1;
		node->m_velocity0 = node->m_velocity;
		node->m_velocity = factor2*x2 + factor1*x1 + factor0*x0;
		x0 = x1, x1 = x2;
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}


//====Newmark====================================
NewmarkIntegrator::NewmarkIntegrator(
	CSimuEntity *pobj, const double *_pmass, const int stridemass):
	ODEIntegrator(pobj)
{
	m_bIsImplicit = true;
	const int nv = pobj->getVertexCount();
	this->resize(nv);

	//IC preconditioner 
	m_pPrecondMatrix = NULL;

	const double *pmass = _pmass;
	double totalmass = 0;
	for (int i=0; i<nv; i++, pmass+=stridemass){
		m_mass[i] = *pmass;
		totalmass += *pmass;
	}
	m_avgmass = totalmass/nv;
	printf("Vertex num %d, avg mass %lg.\n", nv, m_avgmass);
}


NewmarkIntegrator::~NewmarkIntegrator(void)
{
	if (m_pPrecondMatrix) 
		delete m_pPrecondMatrix, m_pPrecondMatrix=NULL;
}


void NewmarkIntegrator::resize(const int nv)
{
	const unsigned int nfloat = 9*nv;
	if (m_vbuffer.size()<nfloat) m_vbuffer.resize(nfloat);

	if (m_mass.size()<nv) m_mass.resize(nv);

	//allocate buffer
	const int stiffnesstype = m_pObj->isStiffnessMatrix3X3()?1:0;
	if (m_pSitffnessMat==NULL)
		m_pSitffnessMat = new CSparseMatrix33(nv, stiffnesstype);
	else if (m_pSitffnessMat->size()!=nv){
		delete m_pSitffnessMat;
		m_pSitffnessMat = new CSparseMatrix33(nv, stiffnesstype);
	}
	else
		m_pSitffnessMat->init(nv);
	ASSERT0(m_pSitffnessMat!=NULL);

	//copy
	m_nv = nv;
}


//velocity verlet algorithm == explicit Newmark
//This function implements the semi-implicit Newmark alpha method, I also added the 
//trapozodial rule in the displacement part, eg. x^(n+1) = h[v^n+v^(n+1)], and
//Ma^n = f^n, Ma^(n+1)=f^(n+1)+(1-alpha)(f^(n+1)-f^n)
void NewmarkIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double dt05 = 0.50*dt;
	const double dt05dt = dt05*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	const int nv = obj.getVertexCount();	
	const bool conststiff = obj.hasConstantStiffness();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	if (timeid==0){
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;		//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			x += dt*v + a*dt05dt, v += dt*a;
		}
		goto COMPUTE_ACC;
	}

	//setup Newmark constants
	const double alpha = -0.050;
	const double alpha1 = 1.0-alpha;
	const double gamma = 0.5-alpha;
	const double beta = alpha1*alpha1*0.25;
	const double beta2 = 2.0*beta;
	const double kk = 1.0/(alpha1*dt*beta2);	
	const double k1 = -kk/(dt*gamma);
	const double k2 = -kk;
	const double k3 = -kk*(1.0-gamma)/gamma;
	const double k4 = -1.0/beta2;
	const double k5 = k1*k4;
	const double k2k3 = k2+k3;

	//Solve eqn AX = B, first compute right term A
	const double* pmass = this->getMassBuffer();
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	if ((!conststiff) || (timeid==1)) A.addDiagonalMatrix(pmass, k1);
	//then the right part b vector
	Vector3d* b = (Vector3d*)this->getVertexArrayBuffer();
	Vector3d* b0 = b+nv;
	for (int i=0; i<nv; i++) b0[i]=pnodearray[i].m_velocity*k4;
	A.multiplyVector(b0, b);
	for (int i=0; i<nv; i++)
		b[i] += k2k3*pnodearray[i].m_force - (pmass[i]*k5)*pnodearray[i].m_velocity;

	//solve the EQ, find delta V, where initial guess is zero (set in CG alg.)
	int nitr;
	Vector3d *dV = b0;
	const double eps=1.0e-5; const int maxiter=25;
	if (conststiff){ //Incomplete Cholesky preconditioner
		if (m_pPrecondMatrix==NULL){
			A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, dV, eps, maxiter);
	}
	else{ //build diagonal preconditioner
		Vector3d* const C = dV + nv;				
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, dV, eps, maxiter);
		A.clear();
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaV
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *n = &pnodearray[i];
		n->m_pos += dt*(n->m_velocity + beta2*dV[i]);
		n->m_velocity += dV[i];
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}


void ImplicitGMPIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double dt05 = 0.50*dt;
	const double dt05dt = dt05*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	const int nv = obj.getVertexCount();	
	const bool conststiff = obj.hasConstantStiffness();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	if (timeid==0){
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;		//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			x += dt*v + a*dt05dt, v += dt*a;
		}
		goto COMPUTE_ACC;
	}

	//setup Newmark constants
	const double k = 1;
	const double rou = 0.9980;
	const double K1 = -(1+rou)*(1+rou)/(k*dt*dt);
	const double K2 = K1*dt;
	const double K3 = K1*dt*dt/(1+rou);

	//Solve eqn AX = B, first compute right term A
	const double* pmass = this->getMassBuffer();
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	//then the right part b vector
	Vector3d* b = (Vector3d*)this->getVertexArrayBuffer();
	Vector3d* b0 = b+nv;
	for (int i=0; i<nv; i++) b0[i]=pnodearray[i].m_velocity*K3;
	A.multiplyVector(b0, b);
	for (int i=0; i<nv; i++) b[i] += K2*pnodearray[i].m_force;
	A.addDiagonalMatrix(pmass, K1);

	//solve the EQ, find delta V, where initial guess is zero (set in CG alg.)
	int nitr;
	Vector3d *dV = b0;
	const double eps=1.0e-5; const int maxiter=25;
	if (conststiff){ //Incomplete Cholesky preconditioner
		if (m_pPrecondMatrix==NULL){
			A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, dV, eps, maxiter);
		A.addDiagonalMatrix(pmass, -K1);
	}
	else{ //build diagonal preconditioner
		Vector3d* const C = dV + nv;				
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, dV, eps, maxiter);
		A.clear();
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaV
	const double K4 = dt/(1+rou);
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *n = &pnodearray[i];
		n->m_pos += dt*n->m_velocity + K4*dV[i];
		n->m_velocity += dV[i];
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}


/*
//velocity verlet algorithm == explicit Newmark
void NewmarkIntegrator::doIntegration(const unsigned int timeid, const double& _dt)
{
	CSimuEntity &obj = *m_pObj;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double dt05 = 0.50*dt;
	const double dt05dt = dt05*dt;
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	const int nv = obj.getVertexCount();	
	const bool conststiff = obj.hasConstantStiffness();
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	if (timeid==0){
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;		//give alias names
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			x += dt*v + a*dt05dt;			//advance x one step using the method in velocity verlet
		}
		goto COMPUTE_ACC;
	}

	//setup Newmark constants
	const double alpha = -0.100;
	const double gamma = 0.5-alpha;
	const double gamma1 = 1.0-gamma;
	const double kv1 = dt*gamma1;
	const double kv2 = dt*gamma;
	const double beta = (1-alpha)*(1-alpha)*0.25;
	const double kx2 = dt05dt*2.0*beta;
	const double kx1 = dt05dt-kx2;
	const double alpha1 = 1.0-alpha;
	const double k1 = alpha1*dt;
	const double k2 = alpha1*kx1;
	const double k3 = alpha1*kx2;
	const double k13 = -k1/k3;
	const double k23 = -k2/k3;
	const double ik3 = -1.0/k3;

	//Solve eqn AX = B, first compute right term B
	CSparseMatrix33 &A = *(this->getSparseMatrix());
	Vector3d* const b0 = (Vector3d*)this->getVertexArrayBuffer();
	for (int i=0; i<nv; i++) b0[i] = pnodearray[i].m_velocity*k13 + pnodearray[i].m_acc*k23;
	Vector3d* const b = b0 + nv;	
	A.multiplyVector(b0, b);
	for (int i=0; i<nv; i++) b[i] += pnodearray[i].m_force*ik3;

	//Compute pF/pX - M/h^2, prepare left term A
	const double* pmass = this->getMassBuffer();
	A.addDiagonalMatrix(pmass, ik3);
	//for (int i=0; i<nv; i++) b0[i] = pnodearray[i].m_acc;
	//Vector3d* const C = b + nv;	
	//A.multiplyVector(b0, C);
	//for (int i=0; i<nv; i++) b[i]-=C[i];

	//solve the EQ, find a^(n+1), where initial guess is zero (set in CG alg.)
	int nitr;
	Vector3d *acc = b0;
	const double eps=1.0e-3; const int maxiter=25;
	if (conststiff){ //Incomplete Cholesky preconditioner
		if (m_pPrecondMatrix==NULL){
			//A.deleteZeroElements();
			m_pPrecondMatrix = new CICPrecondMatrix(A);
		}
		nitr = SparseSolverCghs(A, *m_pPrecondMatrix, b, acc, eps, maxiter);
		A.addDiagonalMatrix(pmass, -ik3);
	}
	else{ //build diagonal preconditioner
		Vector3d* const C = b + nv;				
		for (int i=0; i<nv; i++) C[i].x=C[i].y=C[i].z=1.0/pmass[i];
		nitr = SparseSolverCghs(A, C, b, acc, eps, maxiter);
	}
	m_nTotalIterationCount += nitr;
	//printf("%d:CG itr# %d.\n", timeid, nitr);

	//update results using deltaV
	for (int i=0; i<nv; i++){
		CSimuEntity::VertexInfo *n = &pnodearray[i];
		n->m_pos += dt*n->m_velocity + kx1*n->m_acc + acc[i]*kx2;
		n->m_velocity += kv1*n->m_acc + kv2*acc[i]; 
	}

COMPUTE_ACC:
	//apply position constraint filters, note that time is in milisec.
	obj.performEdgeRelaxation(current_tm, dt1000);
	
	//update force and stiffness
	obj.computeAcceleration(timeid, !conststiff);
}
*/
