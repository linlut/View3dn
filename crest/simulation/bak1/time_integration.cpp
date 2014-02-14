//FILE: time_integration.cpp

#include "time_integration.h"
#include "simu_entity.h"
#include "sparseMatrix.h"
#include "CGSolver.h"
#include "zsparse_matrix.h"


void VelocityVerletIntegration(CSimuEntity &obj, const int timeid, const double& dt)
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


void VelocityBasedIntegration(CSimuEntity &obj, const int timeid, const double& dt)
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
	obj.updateState();
}

static inline double computeAccDamp(const Vector3d &a, const double &kdamp)
{
	//return kdamp;
	const double K = 1E-2;
	const double c = Magnitude2(a);
	const double r = kdamp * c / (c+K);
	return r;
}
void VerletIntegration(CSimuEntity &obj, const int timeid, const double& dt)
{
	const double dtdt = dt*dt;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();

	//get vel damping, clamp to max 0.5;
	const CMeMaterialProperty* pmat = obj.getMaterial();
	const double vdamp_ratio = pmat->getMassDamping();

	//Advect the location
	//at the very begining we have difficulty to use the above equation
	//therefore, we just approximate the first step as
	if (timeid>0){
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = node->m_tmp0;			
			Vector3d &x2 = node->m_tmp1;			
			Vector3d &a = node->m_acc;
			x2 = x1;							//backup pos in t-1
			x1 = x;								//backup pos in t
			const double vdampK = vdamp_ratio;  //computeAccDamp(a, vdamp_ratio);
			const double vdamp2  = 2.0 - vdampK;
			const double vdamp1  = 1.0 - vdampK;
			x = x1*vdamp2 - vdamp1*x2 + dtdt*a;	//update x
		}
	}
	else{//the initial time step =0 is advanced in a simple way
		const double dtdt05 = 0.5*dtdt;
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = node->m_tmp0;			
			Vector3d &v = node->m_velocity;			
			Vector3d &a = node->m_acc;
			x1 = x;								//backup pos in t
			x += v*dt + a*dtdt05;				//update x
		}
	}

	//apply position constraint filters, note that time is in milisec.
	const double dt1000 = dt*1000.0;
	const double current_tm = dt1000 * timeid;
	obj.performEdgeRelaxation(current_tm, dt1000);

	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid);
}

/*
static inline double computeAccDamp(const Vector3d &a, const double &k)
{
	const double K = 100000;
	const double c = Magnitude2(a);
	const double r =  (1.0 -k) * K /(c + K);
	return 1;
}

void VerletIntegration(CSimuEntity &obj, const int timeid, const double& dt)
{
	double dtdt = dt*dt;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();

	//get vel damping, clamp to max 0.5;
	const CMeMaterialProperty* pmat = obj.getMaterial();
	double vdamp_ratio = pmat->getVelocityDamping();
	if (vdamp_ratio>0.50) vdamp_ratio = 0.50;
	dtdt *=(1.0-vdamp_ratio);

	//Advect the location
	//at the very begining we have difficulty to use the above equation
	//therefore, we just approximate the first step as
	if (timeid>0){
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = node->m_tmp0;			
			Vector3d &x2 = node->m_tmp1;			
			Vector3d &a = node->m_acc;
			x2 = x1;							//backup pos in t-1
			x1 = x;								//backup pos in t
			const double Kdamp = computeAccDamp(a, 0);
			x = x1 + x1- x2 + (dtdt*Kdamp)*a;	//update x
		}
	}
	else{//the initial time step =0 is advanced in a simple way
		for (int i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			Vector3d &x = node->m_pos;			
			Vector3d &x1 = node->m_tmp0;			
			Vector3d &v = node->m_velocity;			
			Vector3d &a = node->m_acc;
			x1 = x;								//backup pos in t
			x += v*dt+a*(0.5*dtdt);				//update x
		}
	}

	//apply position constraint filters, note that time is in milisec.
	const double current_tm = 1000.0* dt * timeid;
	const double dt1000 = dt*1000.0;
	obj.performEdgeRelaxation(current_tm, dt1000);

	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid);
}
*/

static inline Vector3d ComputeDeltaV(
	const double &m, const double& invh, const Vector3d& a0, const Vector3d& v0, const double3x3 &jac)
{
	const double m4h =4.0*m*invh;
	const double m4hh =m4h*invh;
	const Vector3d v02 = v0+v0;
	const Vector3d term1 = -(m4h*a0+jac*v02);
	double3x3 term2 = jac;
	term2.x[0]-=m4hh;
	term2.x[4]-=m4hh;
	term2.x[8]-=m4hh;
	term2.Invert();
	Vector3d dv=term2*term1;
	return dv;
}

void ImplicitRectangularIntegration(CSimuEntity &obj, const int timeid, const double& dt)
{
	int i;
	const int LOOPNUM=4;
	const double invdt = 1.0/dt;
	const double dt05 = 0.5*dt;
	const double dt05dt = dt05*dt;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints()){
			Vector3d &x = node->m_pos;
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			Vector3d &x0 = node->m_tmp0;
			Vector3d &v0 = node->m_tmp1;
			Vector3d &a0 = node->m_tmp2;
			x0=x, v0=v, a0=a;					//backup x, v, a
			x = x0+ dt05*v0; //*dt + a0*dt05dt);	    //advance x one step using the method in velocity verlet
		}
	}

	for (int j=0; j<LOOPNUM; j++){
		//Update the acceration at the new time step, acc is updated
		obj.computeAcceleration(timeid, true);
		//update velocity
		for (i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			if (node->m_constraints.hasNoConstraints()){
				Vector3d &x = node->m_pos;
				Vector3d &v = node->m_velocity;
				const Vector3d &x0 = node->m_tmp0;
				const Vector3d &v0 = node->m_tmp1;
				const Vector3d &a0 = node->m_tmp2;
				//const Vector3d deltaV=ComputeDeltaV(node->m_mass, invdt, a0, v0, node->m_jac);
				//v=v0+deltaV;
				x=x0+dt05*(v0+v);
			}
		}
	}

	//update state vars
	obj.updateState();
}


/*
void ImplicitEulerIntegration(CSimuEntity &obj, const int timeid, const double& dt)
{
	int i;
	const double invdt = 1.00/dt;
	const double ninvdt = -1.00/dt;
	const double ninvdtdt = -1.00/(dt*dt);
	const double dt05 = 0.50*dt;
	const double dt05dt = dt05*0.50;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	
	//initial step;
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints()){
			Vector3d &x = node->m_pos;
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			Vector3d &f = node->m_force;
			Vector3d &x0 = node->m_tmp0;
			Vector3d &v0 = node->m_tmp1;
			Vector3d &a0 = node->m_tmp2;
			Vector3d &f0 = node->m_tmp3;
			x0=x, v0=v, a0=a, f0=f;				//backup x, v, a, f
			x = x0; //+ dt05*v0;				// + a0*dt05dt;	//advance x one step using the method in velocity verlet
		}
		else{
			node->m_mass = 1e36;
			node->m_velocity = 0;
			node->m_tmp3 = 0;
		}
	}

	//find the deltaV
	//Update the acceration at the new time step, acc is updated
	obj.computeAcceleration(timeid, true);
	SparseMatrix A(obj.m_pImplicitSolverData->m_pSparseMat);

	//
	Vector3d *V = obj.m_pImplicitSolverData->m_pVecBuffer0;
	Vector3d *F = obj.m_pImplicitSolverData->m_pVecBuffer1;
	Vector3d *B = F;
	for (i=0; i<nv; i++){
		const Vector3d& v0 = pnodearray[i].m_tmp1;
		V[i] = v0 * dt;
	}
	A.MultiplyVector((double*)V, (double*)F);
	for (i=0; i<nv; i++){
		const Vector3d &f0 = pnodearray[i].m_tmp3;
		B[i]= (F[i] + f0)*ninvdt;
	}

	//Compute pF/pX - M/h^2
	Vector3d *M = obj.m_pImplicitSolverData->m_pVecBuffer0;
	for (i=0; i<nv; i++){
		const double t = pnodearray[i].m_mass * ninvdtdt;
		M[i].x = M[i].y = M[i].z = t;
	}
	A.AddDiagonalMatrix((double*)M);

	//solve the equation 
	CGSolver cgsolver(&A);
	Vector3d *dV = obj.m_pImplicitSolverData->m_pVecBuffer0;
	for (i=0; i<nv; i++){ //put initial guess on the output buffer, required by the CG program here
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints())
			dV[i] = pnodearray[i].m_tmp2 * dt05;
		else
			dV[i] = Vector3d(0);
	}
	const double eps=1e-6;
	const int maxIter=80;
	int nitr = cgsolver.CGSolve((double*)B, (double*)dV, eps, maxIter);
	if (nitr>=maxIter){
		printf("CG alg. is not converge at %d time step \n\n", timeid);
	}

	//update velocity
	double v2;
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints()){
			Vector3d &x = node->m_pos;
			Vector3d &v = node->m_velocity;
			Vector3d &x0 = node->m_tmp0;
			Vector3d &v0 = node->m_tmp1;
			v=v0+dV[i];
			x=x0+dt05*(v+v0);
			v2 = Magnitude2(dV[i]-Vector3d(0,0,-0.01));
			if (v2>1e-10){
				v2=100;
				int x=1;
			}
		}
	}

	//update state vars
	obj.updateState();
}
*/

void ImplicitEulerIntegration(CSimuEntity &obj, const int timeid, const double& _dt)
{
	int i;
	double dt = _dt;
	if (timeid==0) dt = 1e-8; //use a small dt for start up step
	const double invdt = 1.00/dt;
	const double ninvdt = -1.00/dt;
	const double ninvdtdt = -1.00/(dt*dt);
	const double dt05 = 0.50*dt;
	const double dt05dt = dt05*0.50;
	const int nv = obj.getVertexCount();	
	CSimuEntity::VertexInfo *pnodearray = obj.getVertexInfoPtr();
	const Vector3d ZEROVEC3D(0,0,0);

	//initial step;
	if (timeid==0){
		for (i=0; i<nv; i++){
			CSimuEntity::VertexInfo *node = &pnodearray[i];
			if (node->m_constraints.hasNoConstraints()){
				Vector3d &x = node->m_pos;		//give alias names
				Vector3d &v = node->m_velocity;
				Vector3d &a = node->m_acc;
				Vector3d &f = node->m_force;
				Vector3d &x0 = node->m_tmp0;
				Vector3d &v0 = node->m_tmp1;
				Vector3d &a0 = node->m_tmp2;
				Vector3d &f0 = node->m_tmp3;
				x0=x, v0=v, a0=a, f0=f;			//backup x, v, a, f
				x = x0 + dt05*v0 + a0*dt05dt;	//advance x one step using the method in velocity verlet
			}
			else{
				node->m_mass = 1e3;
				node->m_velocity = ZEROVEC3D;
				node->m_force = ZEROVEC3D;
			}
		}
		goto COMPUTE_ACC;
	}

	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints()){
			Vector3d &x = node->m_pos;
			Vector3d &v = node->m_velocity;
			Vector3d &a = node->m_acc;
			Vector3d &f = node->m_force;
			Vector3d &x0 = node->m_tmp0;
			Vector3d &v0 = node->m_tmp1;
			Vector3d &a0 = node->m_tmp2;
			Vector3d &f0 = node->m_tmp3;
			x0=x, v0=v, a0=a, f0=f;			//backup x, v, a, f
		}
		else{
			node->m_mass = 1e3;
			node->m_velocity = ZEROVEC3D;
			node->m_force = ZEROVEC3D;
		}
	}
	//find the deltaV
	//Update the acceration at the new time step, acc is updated
	CSparseMatrix33 &A = *(obj.m_pImplicitSolverData->m_pSparseMat33);
	Vector3d *V = obj.m_pImplicitSolverData->m_pVecBuffer0;
	Vector3d *F = obj.m_pImplicitSolverData->m_pVecBuffer1;
	Vector3d *B = F;
	for (i=0; i<nv; i++) V[i] = pnodearray[i].m_tmp1;
	A.multiplyVector(V, F);
	for (i=0; i<nv; i++){
		const Vector3d &f0 = pnodearray[i].m_tmp3;
		B[i]= f0*ninvdt-F[i];
	}

	//Compute pF/pX - M/h^2, prepare term 
	Vector3d *M = obj.m_pImplicitSolverData->m_pVecBuffer0;
	for (i=0; i<nv; i++){
		double t = pnodearray[i].m_mass * ninvdtdt;
		M[i].x = M[i].y = M[i].z = t;
	}
	A.addDiagonalMatrix(M);

	//put initial guess on the output buffer, required by the itr program
	Vector3d *dV = obj.m_pImplicitSolverData->m_pVecBuffer0;
	for (i=0; i<nv; i++){ 
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints())
			dV[i] = pnodearray[i].m_tmp2 * dt05;
		else
			dV[i] = Vector3d(0,0,0);
	}

	//solve the equation 
	const double eps=1e-6;
	const int maxIter=33;
	//int nitr = SparseSolverCghs(A, B, dV, eps);
	int nitr = SparseSolverBicgsq(A, B, dV, eps);
	//int nitr = SparseSolverGmres(A, B, dV, eps);
	//int nitr = SparseSolverBicgstab(A, B, dV, eps);
	//int nitr = solver.solveJacobi(A, B, dV, eps, maxIter);
	//if (nitr>=maxIter)
	//	printf("Jacobi alg. is not converge at time step: %d.\n\n", timeid);

	//update velocity and position
	for (i=0; i<nv; i++){
		CSimuEntity::VertexInfo *node = &pnodearray[i];
		if (node->m_constraints.hasNoConstraints()){
			Vector3d &x = node->m_pos;
			Vector3d &v = node->m_velocity;
			Vector3d &x0 = node->m_tmp0;
			Vector3d &v0 = node->m_tmp1;
			v=v0+dV[i];
			x=x0+dt*v;
		}
	}

	//compute acc & update state vars
COMPUTE_ACC:
	if (obj.m_pImplicitSolverData ==NULL)
		obj.m_pImplicitSolverData = new CImplicitSolverData();
	ASSERT0(obj.m_pImplicitSolverData!=NULL);
	obj.m_pImplicitSolverData->update(nv);
	obj.computeAcceleration(timeid, true);
	obj.updateState();
}

