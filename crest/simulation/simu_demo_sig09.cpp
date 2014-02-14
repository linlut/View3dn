//FILE: simu_engine.cpp
#include <memory.h>
#include <sysconf.h>
#include <ztime.h>
#include "simu_engine.h"
#include "simu_thinshell.h"
#include "simu_thinshell2.h"
#include "simu_tetra2.h"
#include "simu_springobj.h"
#include "simu_staticrod.h"

#include "constraints.h"


void test_mainCLDRAPE(void)  //CL_DRAPE_SPH CLOTH drape on sphere
{
	//typedef CSimuSpringThinshell T;
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setDensity(2e+2);
	mtl.setYoung(2.500e4);
	mtl.setPoisson(0.300);
	mtl.setDamping(0.3, 0);
	mtl.m_rigid =3;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(2.00E-3);	
	//T *s = T::genShellFromFile("clothweb20x20.quad", mtl);
	T *s = T::genShellFromFile("cloth99x99.quad", mtl);

	//set constraints
	const double r = 4.50;
	const double y = -r+ 0.5;
	const Vector3d center(0, y, 0); 
	CSphereConstraint* sphcons = new CSphereConstraint(10, 0, 1e10, -1, NULL, r, center);
	s->addConstraint(sphcons);
	s->setGravityAccelertion(Vector3d(0, -10, 0));

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(5.00E-3); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(20000, 400);
	//e.startSimulation(800, 2);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
}


void test_mainPLCONS(void)	//TEST PLANE constraint  PL_CONS
{
	typedef CSimuSpringThinshell T;
	//typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(4.00e7);
	mtl.setPoisson(0.30);
	mtl.setDensity(1.00e3);
	mtl.setDamping(1e-2, 1e-2);
	mtl.m_rigid =0;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(5.0000E-4);
	T *s = T::genShellFromFile("box04.txt", mtl);

	//set constraints
	double eqn[4]={0, 0, 1, 15};
	CPlaneConstraint* sphcons = new CPlaneConstraint(10, 0, 1e10, -1, NULL, eqn);
	s->addConstraint(sphcons);
	s->setGravityAccelertion(Vector3d(0, 0, -10));

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(1.00E-3); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(20000, 100);
	//e.startSimulation(800, 2);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
}


void test_mainCOLLL(void)	//COLLISION RESPONSE
{
	typedef CSimuSpringThinshell T;
	//typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(1.00e8);
	mtl.setPoisson(0.30);
	mtl.setDamping(1e-2, 1e-2);
	mtl.m_rigid =0;	
	mtl.setMassDamping(1E-3);
	T *s = T::genShellFromFile("box04.txt", mtl);

	//set constraints
	const double tt = 33;
	CCollisionConstraint* col = new CCollisionConstraint(tt);

	Vector3d pVertexPos(-6, -6, -6);
	int pVertexID=7;
	const int nv=1;
	const double cur_time=0;
	s->updateCollisionConstraint(&pVertexPos, &pVertexID, nv, cur_time, tt);

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(0.500E-3); // max step for verlet
	e.startSimulationInMiliSeconds(1000, 10);
}


void test_mainVVV(void)  //V shaped Beam
{
	//typedef CSimuSpringThinshell T;
	const double gap = 0.25;
	const double h =0.185 / 2;
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	const double K = 10;
	mtl.setYoung(10.00e5 * K*K);
	mtl.setPoisson(0.30);
	mtl.setDamping(0.5000, 0.100);
	mtl.m_rigid = 0;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(5.00E-4);	

	CSimuEngine e;
	const double tmstep = 1.00e-3/K;
	e.setTimestep(tmstep); // max step for verlet

	{
		T *s = T::genShellFromFile("beamsurf4-32.quad", mtl);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	{
		Vector3d trans(gap, h, 0); 
		T *s = T::genShellFromFile("beamsurf4-32H.quad", mtl, &trans);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	//=========================================================================
	{
		CMeMaterialProperty mtl0 = mtl;
		mtl0.setYoung(mtl.getYoung()/K);
		Vector3d trans(1.0, -0.0, 0); 
		T *s = T::genShellFromFile("beamsurf4-32.quad", mtl0, &trans);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	{
		Vector3d trans(+1+gap, h, 0); 
		CMeMaterialProperty mtl0 = mtl;
		mtl0.setYoung(mtl.getYoung()/K);
		T *s = T::genShellFromFile("beamsurf4-32H.quad", mtl0, &trans);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	//=========================================================================
	{
		CMeMaterialProperty mtl0 = mtl;
		mtl0.setYoung(mtl.getYoung()/K/K);
		Vector3d trans(2.0, -0.0, 0); 
		T *s = T::genShellFromFile("beamsurf4-32.quad", mtl0, &trans);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	{
		CMeMaterialProperty mtl0 = mtl;
		mtl0.setYoung(mtl.getYoung()/K/K);
		Vector3d trans(2.0+gap, +h, 0); 
		T *s = T::genShellFromFile("beamsurf4-32H.quad", mtl0, &trans);
		//set constraints
		int fixednodes[]={0, 1, 199, 2, 200, 201, 202, 203, 234, 265, 296, 3, 4, 5, 6, 7, 8, 9}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(Vector3d(0,-10, 0));
		e.addSimuObject(s);
	}
	//=========================================================================
	{
		char *wallname = "vbeam-wall.quad";
		T *pwallshell = T::genShellFromFile(wallname, mtl);
		assert(pwallshell!=NULL); 
		e.addSimuObject(pwallshell);
	}


	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(5000, 50);
	//e.startSimulation(40000, 200);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}


void test_mainSHEARLOCK(void)  //SHEAR LOCKING
{
	const char *fname = "quad10.quad";
	//typedef CSimuSpringThinshell T;
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	const Vector3d g(0,0, -10);
	const double K = 1;
	mtl.setYoung(20.00e3 * K*K);
	mtl.setPoisson(0.30);
	mtl.setDamping(45.000, 0.100);
	mtl.m_rigid = 0;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(5.0e-4);	
	
	CSimuEngine e;
	const double tmstep = 1.00e-3/K;
	e.setTimestep(tmstep); // max step for verlet

	int fixednodes[]={0, 6, 12, 1, 7, 13};
	{
		const  int isstatic = 1;
		Vector3d trans(+0.0, +0.65, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);
		//set constraints
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(g);
		e.addSimuObject(s);
	}
	{
		const  int isstatic = 0;
		Vector3d trans(+0.0, +0.150, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);
		//set constraints
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(g);
		e.addSimuObject(s);
	}

	//=========================================================================
	{
		char *wallname = "quad10-wall.quad";
		T *pwallshell = T::genShellFromFile(wallname, mtl);
		assert(pwallshell!=NULL); 
		e.addSimuObject(pwallshell);
	}


	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(10000, 100);
	//e.startSimulation(40000, 200);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}



void test_mainSHEARET(void)  //ShearRECT
{
	typedef CSimuStaticRod T;
	CMeMaterialProperty mtl;
	mtl.setYoung(2.00e5);
	mtl.setPoisson(0.40);
	mtl.setDamping(50.00, 0.0);
	mtl.setMassDamping(1.0e-4);
	int nv=4, nelm=4;
	Vector3d pVertex[]={
		Vector3d(0,0,0), Vector3d(1,0,0),
		Vector3d(0,1,0), Vector3d(1,1,0),	
	};		
	Vector3d init_velocity(0,0,0);
	int element[8]={0,1, 0,2, 3, 2, 3, 1};
	const double rod_crossarea=1e-5;
	T *s = new T(pVertex, nv, init_velocity, element, nelm,	rod_crossarea, mtl);

	//set constraints
	{//fixed pos.
		int fixednodes[]={0,1}; 
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
		s->addConstraint(fixpos);
		s->setGravityAccelertion(Vector3d(0,0,0));
	}

	const double dx = 0.999;
	const double dy = -1.0;
	const double dz= 0.0;
	{
		int movenodes[]={2, 3};
		Vector3d vel(dx, dy, dz); vel*=1.00e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(10, 1, 1000, 2, movenodes, vel);
		s->addConstraint(velo);
	}
	{
		int movenodes[]={2, 3};
		Vector3d vel(-dx, -dy, -dz); vel*=1.000e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(20, 1001, 2000, 1, movenodes, vel);
		//s->addConstraint(velo);
	}

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(2.00e-4); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(10000, 50);
	//e.startSimulation(100, 1);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	s->exportElementState(0, mtl, stdout);
}


void test_mainSPEED(void)
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
	const int N=10000000;
	const double K = 360.0/N;
	const double A1 = (PI*2.0)*K/360.0;
	const int timerid=0;

	startFastTimer(timerid);
	for (int i=0; i<=N; i++){
		Vector3d Force;
		double x = 1;
		double y = 0;
		p0=Vector3d(x, y, 0);
		e.computeNodalForce(p0, p1, mat, Force, NULL);
		//const double f2 = Magnitude(Force);
		//const double engr = e.computeElementEnergy(mat);
		//printf("%d, %lg, %lg, %lg, %lg, %lg\n", i*K, Force.x, Force.y, Force.z, f2, engr);
	}
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Shear element run time is ");
	printf("SHear elem number is %d\n", N);
}




void test_mainRRR(void)
{
	typedef CSimuStaticRod T;
	CMeMaterialProperty mtl;
	mtl.setYoung(2.00e5);
	mtl.setPoisson(0.4);
	mtl.setDamping(50.0, 0.0);
	mtl.setMassDamping(1.0e-4);
	int nv=2; 
	int nelm=1;
	const double z1 = 0.02;
	Vector3d pVertex[]={
		Vector3d(2,0,0),  Vector3d(3,0,0), 
	};		
	Vector3d init_velocity(0,0,0);
	int element[4]={0,1, 2, 3};
	const double rod_crossarea=1e-6;
	T *s = new T(pVertex, nv, init_velocity, element, nelm, rod_crossarea, mtl);	

	//set constraints
	int fixednodes[]={0}; 
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s->addConstraint(fixpos);
	//s->setGravityAccelertion(Vector3d(0,-10,0));

	{
		int movenodes[]={1};
		Vector3d vel(sqrt(2.0)-1, 0, -1); vel*=1e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(10, 1, 1000, 1, movenodes, vel);
		//s->addConstraint(velo);
	}
	{
		const Vector3d center=pVertex[0];
		const Vector3d norm(0,1,0);
		int vertbuffer[]={1};
		const double av = PI*1.0*1e-3 *0.50;//0.25000;
		CPlanarRotationConstraint *r = new CPlanarRotationConstraint(
			10, 0, 1000,
			1, vertbuffer, 
			center, norm, av);
		s->addConstraint(r);
	}
	{
		int fixednodes[]={1};
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(15, 1001, 1500, len, fixednodes);
		//s->addConstraint(fixpos);
	}

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(10.00e-5); // max step for verlet
	e.startSimulationInMiliSeconds(10000, 50);
	s->exportElementState(0, mtl, stdout);
}


void test_mainCACTUS(void)  //SHEAR LOCKING
{
	const char *fname = "c:/users/nanzhang/data/obj/cactus.txt";
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	const Vector3d g(0,1.75, 0);
	const double K = 1;
	mtl.setYoung(15.00e4 * K*K);
	mtl.setPoisson(0.30);
	mtl.setDamping(30.000, 0.100);
	mtl.m_rigid = 1;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(3.0e-4);	
	
	CSimuEngine e;
	const double tmstep = 1.00E-4;
	e.setTimestep(tmstep); // max step for verlet

	int fixednodes[]={ 314 ,331,332 ,356 ,357 ,576 ,577,588 ,596 ,604 ,610 ,614 ,616 ,618 ,619 };
	{
		const  int isstatic = 0;
		Vector3d trans(+0.0, +0.0, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);
		//set constraints
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(g);
		e.addSimuObject(s);
	}
	{
		const  int isstatic = 1;
		Vector3d trans(+0.0, +1.10, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);
		//set constraints
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e20, len, fixednodes);
		s->addConstraint(fixpos);
		CVertexConstraints fixnode; 
		fixnode.setFixedPositionXYZ();
		for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
		//======================================
		s->setGravityAccelertion(g);
		e.addSimuObject(s);
	}

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(10000, 50);
	//e.startSimulation(10000, 100);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}


void test_mainBUNY(void)  //BUNNY
{
	const char *fname = "c:/users/nanzhang/data/obj/bunny3.txt";
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	const Vector3d g(0,-9.75, 0);
	const double K = 1;
	mtl.setYoung(20.00e4 * K*K);
	mtl.setPoisson(0.30);
	mtl.setDamping(30.000, 0.100);
	mtl.m_rigid = 1;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(2.0e-4);	


	CSimuEngine e;
	const double tmstep = 1.00E-4;
	e.setTimestep(tmstep); // max step for verlet

		const  int isstatic = 0;
		Vector3d trans(+0.0, +0.0, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);

	double eqn[4]={0, 1, 0, 0.1};
	CPlaneConstraint* sphcons = new CPlaneConstraint(10, 0, 1e10, -1, NULL, eqn);
	s->addConstraint(sphcons);
	s->setGravityAccelertion(g);
	e.addSimuObject(s);


	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(15000, 50);
	//e.startSimulation(10000, 100);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}

void test_mainHEAD(void)  //BUNNY
{
	const char *fname = "c:/users/nanzhang/data/obj/head.txt";
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	const Vector3d g(0,0, -9.75);
	mtl.setYoung(80.00e4);
	mtl.setPoisson(0.30);
	mtl.setDamping(30.000, 0.100);
	mtl.m_rigid = 1;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(2.0e-4);	


	CSimuEngine e;
	const double tmstep = 2.00E-4;
	e.setTimestep(tmstep); // max step for verlet

		const  int isstatic = 0;
		Vector3d trans(+0.0, +0.0, 0); 
		T *s = T::genShellFromFile(fname, mtl, &trans, isstatic);

	double eqn[4]={0, 0, 1, 0.4};
	CPlaneConstraint* sphcons = new CPlaneConstraint(10, 0, 1e10, -1, NULL, eqn);
	s->addConstraint(sphcons);
	s->setGravityAccelertion(g);
	e.addSimuObject(s);


	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(15000, 50);
	//e.startSimulation(10000, 100);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}
