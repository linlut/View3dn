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


extern bool loadQuadMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector4i*& pQuad, int &nquad);
extern bool loadTriangleMeshFile(const char *fname, Vector3d*& pVertex, int &nv, Vector3i*& pQuad, int &nquad);
static CSimuThinshell *genShellFromFile(const char *fname, const CMeMaterialProperty &mtl, const int isstatic=false)
{
	Vector3d *pVertex;		
	Vector3i *element;
	Vector4i *qelement;
	Vector3d init_velocity(0,0,0);
	int nv_per_elm, nv, nelm, *pelm;
	const double shell_thickness=0.001;
	const int len = strlen(fname);
	bool istrimesh = false;
	if (strstr(fname+(len-4), ".txt")!=NULL){
		istrimesh=true;
	}
	if (istrimesh){
		nv_per_elm=3;
		bool r =loadTriangleMeshFile(fname, pVertex, nv, element, nelm);
		pelm = (int*)element;
		if (!r) return NULL;
	}
	else{
		nv_per_elm=4;
		bool r =loadQuadMeshFile(fname, pVertex, nv, qelement, nelm);
		pelm = (int*)qelement;
		if (!r) return NULL;
	}

	CSimuThinshell *pshell = new CSimuThinshell(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		pelm, nv_per_elm, nelm,			//element information
		shell_thickness,
		mtl,							//material property
		isstatic
		);
	return pshell;
}


void test_mainSSS(void)
{
	CMeMaterialProperty mtl;
	mtl.setYoung(2e4);
	mtl.setMassDamping(5e-3);
	mtl.m_rigid = 2;
	mtl.setDamping(0.50, 0.50);
	const int nv=6;
	Vector3d pVertex[nv]={
		Vector3d(0, 0, 0),
		Vector3d(1, 0, 0),
		Vector3d(0, 1, 0),
		Vector3d(1, 1, 0),
		Vector3d(0.5, 1.5, 0),
		Vector3d(0.5, -0.5, 0)
	};
	Vector3d init_velocity(0,0,0);
	const int nelm=4;
	Vector3i element[nelm]={
		Vector3i(0, 1, 3), 
		Vector3i(0, 3, 2),
		Vector3i(2, 3, 4),
		Vector3i(0, 5, 1)
	};
	const int nv_per_elm=3;
	const double shell_thickness=0.001;
	const int isstatic = 0;
	typedef CSimuThinshell T;
	T *pshell = new T(
		pVertex, 6,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, 4,   //element information
		shell_thickness,
		mtl,							//material property
		isstatic
		), &s = *pshell;
	//set constraints
	int fixednodes[]={0, 3};
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s.addConstraint(fixpos);
	s.setGravityAccelertion(Vector3d(0,0,-10));

	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(30e-3);  //max step len for implicit
	//e.setTimestep(70e-3);  //max step len for explicit
	e.startSimulation(10000, 50);

}

void test_mainPLATEPLATETRI(void)		//PLATEPLATETRI
{
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(25.0e4);
	mtl.setMassDamping(1e-4);
	mtl.setDamping(10, 0);	//strain rate damping
	mtl.m_rigid = 0;
	const int nv=18;
	Vector3d pVertex[]={
		Vector3d(0,   0, 0),
		Vector3d(0.1, 0, 0),
		Vector3d(0.2, 0, 0),
		Vector3d(0.3, 0, 0),
		Vector3d(0.4, 0, 0),
		Vector3d(0.5, 0, 0),
		Vector3d(0,   0.1, 0),
		Vector3d(0.1, 0.1, 0),
		Vector3d(0.2, 0.1, 0),
		Vector3d(0.3, 0.1, 0),
		Vector3d(0.4, 0.1, 0),
		Vector3d(0.5, 0.1, 0),
		Vector3d(0,   0.2, 0),
		Vector3d(0.1, 0.2, 0),
		Vector3d(0.2, 0.2, 0),
		Vector3d(0.3, 0.2, 0),
		Vector3d(0.4, 0.2, 0),
		Vector3d(0.5, 0.2, 0),
	};
		
	Vector3d init_velocity(0,0,0);
	const int nelm=20;
	Vector3i element[]={
		Vector3i(0, 1, 6), 
		Vector3i(6, 1, 7),
		Vector3i(1, 2, 7),
		Vector3i(7, 2, 8),
		Vector3i(8, 2, 3),
		Vector3i(8, 3, 9),
		Vector3i(9, 3, 4),
		Vector3i(9, 4, 10),
		Vector3i(10, 4, 5),
 		Vector3i(10, 5, 11),//=============
		Vector3i(12, 6, 13),
		Vector3i(13, 6, 7),
		Vector3i(13, 7, 14),
		Vector3i(14, 7, 8),
		Vector3i(14, 8, 15),
		Vector3i(15, 8, 9),
		Vector3i(15, 9, 16),
		Vector3i(16, 9, 10),
		Vector3i(16, 10, 17),
		Vector3i(17, 10, 11)
	};
	const int nv_per_elm=3;
	const double shell_thickness=0.001;
	T *s = new T(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, nelm,//element information
		shell_thickness,
		mtl								//material property
		);
	s->setGravityAccelertion(Vector3d(0, 0, -10));

	//set constraints
	int fixednodes[]={0, 6, 12, 1, 7, 13};
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s->addConstraint(fixpos);
	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(10.0e-4);
	//======================
	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulation(20000,100);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Plane simmu. run time:");
}

void test_mainPLATEQUAD(void)   //PLATEQUAD
{
	CMeMaterialProperty mtl;
	mtl.setYoung(5.00e4);
	mtl.setDamping(18, 0);	//strain rate damping
	mtl.setMassDamping(1.000e-4);
	mtl.m_rigid = 1;
	const int nv=18;
	const double z0 = 0.1;
	const double z1 = 0.00;
	const double z2 = z0;
	Vector3d pVertex[nv]={
		Vector3d(0,   0, z0),	//node 0
		Vector3d(0.1, 0, z0),
		Vector3d(0.2, 0, z0),
		Vector3d(0.3, 0, z0),
		Vector3d(0.4, 0, z0),
		Vector3d(0.5, 0, z0),
		Vector3d(0,   0.1, z1),
		Vector3d(0.1, 0.1, z1),
		Vector3d(0.2, 0.1, z1),   //node 8
		Vector3d(0.3, 0.1, z1),
		Vector3d(0.4, 0.1, z1),
		Vector3d(0.5, 0.1, z1),
		Vector3d(0,   0.2, z2),
		Vector3d(0.1, 0.2, z2),
		Vector3d(0.2, 0.2, z2),
		Vector3d(0.3, 0.2, z2),
		Vector3d(0.4, 0.2, z2),
		Vector3d(0.5, 0.2, z2),
	};
		
	Vector3d init_velocity(0,0,0);
	const int nelm=10;
	Vector4i element[nelm]={
		Vector4i(0, 1, 7, 6), 
		Vector4i(1, 2, 8, 7),
		Vector4i(2, 3, 9, 8),
		Vector4i(3, 4, 10, 9),
		Vector4i(4, 5, 11, 10),
		Vector4i(6, 7, 13, 12),
		Vector4i(7, 8, 14, 13),
		Vector4i(8, 9, 15, 14),
		Vector4i(9, 10, 16, 15),
		Vector4i(10, 11, 17, 16)
	};
	const int nv_per_elm=4;
	const double shell_thickness=0.004;

	CSimuEngine e;
	//typedef CSimuSpringThinshell T;
	typedef CSimuThinshell T;
	{
		T *pshell = new T(pVertex, nv, init_velocity, &element[0].x, nv_per_elm, nelm, shell_thickness, mtl);
		T &s = *pshell;
		//set constraints
		int fixednodes[]={0, 6, 12, 1, 7, 13};
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e12, len, fixednodes);
		s.addConstraint(fixpos);
		s.setGravityAccelertion(Vector3d(0,0,-10));
		e.addSimuObject(pshell);
	}
	{
		CMeMaterialProperty mtl2 = mtl;
		mtl2.setYoung(mtl.getYoung()*3.0);
		for (int i=0; i<nv; i++) pVertex[i].z+=0.2;
		T *pshell = new T(pVertex, nv, init_velocity, &element[0].x, nv_per_elm, nelm, shell_thickness, mtl2);
		T &s = *pshell;
		//set constraints
		int fixednodes[]={0, 6, 12, 1, 7, 13};
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e12, len, fixednodes);
		s.addConstraint(fixpos);
		s.setGravityAccelertion(Vector3d(0,0,-10));
		e.addSimuObject(pshell);
	}

	e.setTimestep(10.0e-4);
	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(60000,300);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Plane quad simmu. run time:");
}

void test_mainSPHERE(void)  //sphere
{
	//typedef CSimuSpringThinshell T;
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(1.6500e4);
	mtl.setPoisson(0.30);
	mtl.setDamping(0.500, 0.100);
	mtl.m_rigid = 0;	//set the shell to be a rigid shell using relaxation
	mtl.setMassDamping(5.000E-4);	
	T *s = T::genShellFromFile("sphere48.quad", mtl);

	//set constraints
	int fixednodes[]={12, 13, 5,6, 239, 240}; //347, 348, 360, 361
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 4000, len, fixednodes);
	s->addConstraint(fixpos);
	CVertexConstraints fixnode; 
	fixnode.setFixedPositionXYZ();
	for (int i=0; i<len; i++) s->setConstraints(fixednodes[i], fixnode);
	//add a floor constraint
	double eqn[4]={0, 0, 1, 0.275};
	CPlaneConstraint* sphcons = new CPlaneConstraint(10, 0, 1e10, -1, NULL, eqn);
	s->addConstraint(sphcons);
	//======================================
	s->setGravityAccelertion(Vector3d(0,0,-3));

	CSimuEngine e;
	e.addSimuObject(s);
	const double tmstep = 10.00e-4;
	e.setTimestep(tmstep); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(10000, 50);
	//e.startSimulation(800, 2);
	stopFastTimer(timerid);
	const double t = reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
	//printf("Physical time is %lg ms, efficiency %3.1lg\n", tmstep*
}


//Aorta
void test_mainVENA(void)
{
	CMeMaterialProperty mtl;
	mtl.setYoung(3.00e6);
	mtl.setDamping(0.300, 0.300);
	mtl.setMassDamping(6e-5);
	CSimuThinshell *pshell;

	{
	int nv, nelm;
	Vector3d *pVertex, init_velocity(0,0,0);
	Vector4i *element;
	const int nv_per_elm=4;
	const double shell_thickness=0.001;	
	bool r =loadQuadMeshFile("venacava.quad", pVertex, nv, element, nelm); if (!r) return;
	const int isstatic = 0;
	pshell = new CSimuThinshell(
		pVertex, nv,					//vertex array and length
		init_velocity,					//vertex velocity
		&element[0].x, nv_per_elm, nelm,//element information
		shell_thickness,
		mtl,							//material property
		isstatic
		);
	}
	assert(pshell!=NULL); CSimuThinshell &s = *pshell;
	//=============set constraints=========
	int fixednodes[]={365, 717, 718, 334,
		529, 522, 350, 338};
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s.addConstraint(fixpos);

	//=====================================
	s.setGravityAccelertion(Vector3d(0,0,5));
	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(0.5e-3); 

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(20000, 25);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Blood vessel simmu. run time:");
}



void test_mainAORTASTA(void)  //Aorta , for stability problem
{
	typedef CSimuSpringThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(1.0e5);
	mtl.setDamping(1.00, 0.500);
	mtl.setMassDamping(1e-3);
	mtl.m_rigid = 1;	
	T *pshell = T::genShellFromFile("aorta2.quad", mtl);
	assert(pshell!=NULL); T &s = *pshell;
	//=====================================
	s.setGravityAccelertion(Vector3d(0,0,0));
	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(1.00E-3);
	//=====================================
	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulation(100, 1);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Aorta simmu. run time:");
}

void test_mainAORTA(void)		//AORTA
{
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(2.00e5);
	mtl.setDamping(0.250, 0.0);
	mtl.setMassDamping(5.00e-4);
	mtl.m_rigid = 1;	
	T *pshell = T::genShellFromFile("aorta2.quad", mtl);
	assert(pshell!=NULL); T &s = *pshell;
	//========================================================
	int fixednodes[]={309, 310, 311, 312, 313, 314, 315, 316,
					  305 //, 306, 307, 308
					 };
	int rotnodes[]={184, 185,186,187,188,189,190,191}; 

	const int len = sizeof(fixednodes)/sizeof(int);
	const int lenr = sizeof(rotnodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s.addConstraint(fixpos);
	Vector3d rotcenter(0.00, 0.48, 0);
	Vector3d norm(0,-1,0);
	const double av = PI*2.0*1.0e-3;	//angular velocity unit: degree/ms
	CPlanarRotationConstraint* rotpos = new CPlanarRotationConstraint(10, 0, 550, lenr, rotnodes,
		rotcenter, norm, av);
	s.addConstraint(rotpos);
	CFixedPositionConstraint* fixpos2 = new CFixedPositionConstraint(5, 551, 700, lenr, rotnodes);
	s.addConstraint(fixpos2);
	CMouseConstraint * mc  = new CMouseConstraint(309, 33.3);
	s.addConstraint(mc);
	Vector3d mspos(0.05256967, -0.492972, -0.010032078);
	mc->updatePosition(mspos, 0);

	//=====================================
	s.setGravityAccelertion(Vector3d(0,0,0));
	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(5.00e-4); 

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(2000, 5);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "AORTA simmu. run time:");
}


void test_main(void) //BEAMShear
{
	typedef CSimuThinshell T;
	CMeMaterialProperty mtl;
	mtl.setYoung(2.00e4);
	mtl.setPoisson(0.330);
	//mtl.setYoung(5.0e5);
	mtl.setDamping(24.00, 1.00);
	mtl.setMassDamping(1e-4);
	mtl.m_rigid = 0;

	//======================================================
	char *wallname = "beam-wall.quad";
	T *pwallshell = T::genShellFromFile(wallname, mtl);
	assert(pwallshell!=NULL); 
	char *floorname = "beam-floor.quad";
	T *pfloorshell = T::genShellFromFile(floorname, mtl);
	assert(pfloorshell!=NULL); 
	//======================================================
	char *fname = "beam4x4x20.quad";
	int fixednodes[]={0, 1, 2, 3, 4, 105, 106, 107, 108, 109, 225, 226, 227, 
		228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239};
	int rotnodes[]={100,101, 102, 103, 104, 205, 206, 207, 208, 209, 210, 
		211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224}; 
	Vector3d rotcenter(0,0,0.5);
	/*
	char *fname = "beam8x8x40.quad";
	int fixednodes[]={
		 0,1,2,3,4,5,6,7,8, 
		 369,370,371,372,373,374,375,376,377,
		 801,802,803,804,805,806,807,808,809,
		 810,811,812,813,814,815,816,817,818,819,
		 820,821,822,823,824,825,826,827,828,829,
		 830,831,832,833,834,835,836,837,838,839,
		 840,841,842,843,844,845,846,847,848,849,
		 850,851,852,853,854,855,856,857,858,859,
		 860, 861, 862,863
	};
	int rotnodes[]={
		360,361,362,363,364,365,366,367,368,
		729,
		730,731,732,733,734,735,736,737,738,739,
		740,741,742,743,744,745,746,747,748,749,
		750,751,752,753,754,755,756,757,758,759,
		760,761,762,763,764,765,766,767,768,769,
		770,771,772,773,774,775,776,777,778,779,
		780,781,782,783,784,785,786,787,788,789,
		790,791,792,793,794,795,796,797,798,799,
		800
	};
	Vector3d rotcenter(0.1,0.1,1);
	*/
	//=============set constraints=========
	T *pshell = T::genShellFromFile(fname, mtl);
	assert(pshell!=NULL); T &s = *pshell;

	const int len = sizeof(fixednodes)/sizeof(int);
	const int lenr = sizeof(rotnodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e30, len, fixednodes);
	s.addConstraint(fixpos);
	Vector3d norm(0,0,1);
	const double av = PI*1.0E-3*0.650;
	CPlanarRotationConstraint* rotpos = new CPlanarRotationConstraint(10, 0, 2000, lenr, rotnodes, 
		rotcenter, norm, av);
	s.addConstraint(rotpos);
	CFixedPositionConstraint* fixpos2 = new CFixedPositionConstraint(5, 2001, 3000, lenr, rotnodes);
	s.addConstraint(fixpos2);

	//=====================================
	//s.setGravityAccelertion(Vector3d(2, 10, 0));
	CSimuEngine e;
	e.addSimuObject(pshell);
	//e.addSimuObject(pwallshell);
	//e.addSimuObject(pfloorshell);
	e.setTimestep(10.00e-4); // max step for verlet
	//e.setTimestep(1.0e-4); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(18000, 60);
	//e.startSimulation(100, 1);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
}


//FLAG
void test_mainFLAG(void)	//FLAG
{
	typedef CSimuSpringThinshell T;
	CMeMaterialProperty mtl;
	mtl.m_rigid = 3;
	mtl.setYoung(1.00e2);
	mtl.setDamping(0.1, 0.1);
	mtl.setMassDamping(3e-4);
	T *pshell = T::genShellFromFile("cloth.txt", mtl);
	assert(pshell!=NULL);
	T &s = *pshell;

	//set constraints
	CVertexConstraints constraints;
	constraints.setFixedPositionXYZ();
	int fixednodes[]={110, 120}; 
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s.addConstraint(fixpos);
	s.setGravityAccelertion(Vector3d(0,0,-10));

	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(1.0e-2); // max step for verlet

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulationInMiliSeconds(40000, 200);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Hemi sphere simmu. run time:");
}


void test_mainTESTCollision(void)       //TESTCollision
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1.00e4);
	mtl.setDamping(0.0,0.0);
	mtl.setMassDamping(10e-3);
	mtl.m_rigid = 3;
	typedef CSimuSpringThinshell T;
	T *pshell = T::genShellFromFile("sphere-r02.quad", mtl);
	assert(pshell!=NULL);
	T &s = *pshell;
	{
		//int fixednodes[]={0,1, 2, 3};
		int fixednodes[]={67, 69, 70, 79, 80, 83};
		const int len = sizeof(fixednodes)/sizeof(int);
		CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 20000000, len, fixednodes);
		s.addConstraint(fixpos);
	}
	{
		int movenodes[]={2}; //, 1, 45};
		Vector3d vel(0.0, -1.0, 0.0); vel*=2.500e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(10, 1, 200, 1, movenodes, vel);
		//s.addConstraint(velo);
	}
	s.setGravityAccelertion(Vector3d(0,0,0));
	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(3.00E-3);

	const int timerid = 0;
	startFastTimer(timerid);
	const char *fname = "C:/users/nanzhang/project/Test/shell/thinshell/thinshell/collisionlog4.txt";
	e.startSimulationInStepsWithCollisionInput(800, 1, fname);
	//e.startSimulationInSteps(8000, 20);
	s.exportNodeInfo(2);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Plane simmu. run time:");
}


void test_mainxx(void)
{
	CMeMaterialProperty mtl;
	mtl.setYoung(1e9);
	const int nv=24;
	double pVertex[nv*3]={
		-0.00000000033528,-0.00000000033528,0.00000000298023,
		0.02999999966472,-0.00000000033528,0.00000000298023,
		-0.00000000033528,0.02999999966472,0.00000000298023,
		0.02999999966472,0.02999999966472,0.00000000298023,
		-0.00000000033528,-0.00000000033528,0.03000000298023,
		0.02999999966472,-0.00000000033528,0.03000000298023,
		-0.00000000033528,0.02999999966472,0.03000000298023,
		0.02999999966472,0.02999999966472,0.03000000298023,
		-0.00000000033528,-0.00000000033528,0.06000000298023,
		0.02999999966472,-0.00000000033528,0.06000000298023,
		-0.00000000033528,0.02999999966472,0.06000000298023,
		0.02999999966472,0.02999999966472,0.06000000298023,
		-0.00000000033528,-0.00000000033528,0.09000000298023,
		0.02999999966472,-0.00000000033528,0.09000000298023,
		-0.00000000033528,0.02999999966472,0.09000000298023,
		0.02999999966472,0.02999999966472,0.09000000298023,
		-0.00000000033528,-0.00000000033528,0.12000000298023,
		0.02999999966472,-0.00000000033528,0.12000000298023,
		-0.00000000033528,0.02999999966472,0.12000000298023,
		0.02999999966472,0.02999999966472,0.12000000298023,
		-0.00000000033528,-0.00000000033528,0.15000000298023,
		0.02999999966472,-0.00000000033528,0.15000000298023,
		-0.00000000033528,0.02999999966472,0.15000000298023,
		0.02999999966472,0.02999999966472,0.15000000298023
	};
		
	Vector3d init_velocity(0,0,0);
	const int nelm=30;
	int element[nelm*4]={
		1,2,3,5,
		5,6,2,3,
		5,7,6,3,
		2,4,3,6,
		6,8,4,7,
		3,4,7,6,
		5,6,7,9,
		9,10,6,7,
		9,11,10,7,
		6,8,7,10,
		10,12,8,11,
		7,8,11,10,
		9,10,11,13,
		13,14,10,11,
		13,15,14,11,
		10,12,11,14,
		14,16,12,15,
		11,12,15,14,
		13,14,15,17,
		17,18,14,15,
		17,19,18,15,
		14,16,15,18,
		18,20,16,19,
		15,16,19,18,
		17,18,19,21,
		21,22,18,19,
		21,23,22,19,
		18,20,19,22,
		22,24,20,23,
		19,20,23,22
	};
	for (int i=0; i<nelm*4; i++) element[i]--; //back from Fortran input.

	CSimuTetra2* pshell = new CSimuTetra2(
		(const Vector3d*)pVertex, nv,	//vertex array and length
		init_velocity,					//vertex velocity
		element, nelm,					//element information
		mtl								//material property
		);
	CSimuTetra2& s = *pshell;

	//set constraints
	CVertexConstraints constraints;
	constraints.setFixedPositionXYZ();
	s.setConstraints(0, constraints);
	s.setConstraints(1, constraints);
	s.setConstraints(2, constraints);
	s.setConstraints(3, constraints);
	s.setGravityAccelertion(Vector3d(0,-2,0));

	CSimuEngine e;
	e.addSimuObject(pshell);
	e.setTimestep(4.0e-4);

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulation(700);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "Plane simmu. run time:");
}



//void test_main(void)
void test_mainTTTT(void)
{
	CMeMaterialProperty mtl;
	mtl.setYoung(4.0e4);
	int nv, nelm, nface;
	Vector3d* pVertex;
	Vector3d init_velocity(0,0,0);
	int *face, *element;
	int nv_per_elm=4, meshtype;
	const double thickness=0.001;

	bool simLoadNeutralMeshFile(const char *fname, 
	Vector3d *& pVertex, int &nv, 
	int *& pFace, int &ntri,
	int *& pTet, int &ntet, int &meshtype);
	bool r = simLoadNeutralMeshFile("./cubemesh502.mesh", pVertex, nv, face, nface, element, nelm, meshtype);

	CSimuSpringObj *ss = new CSimuSpringObj(
		1,					//springtype, 0: Vangalder, 1: Lloyd
		pVertex, nv,		//vertex array and length	
		init_velocity,		//initial vertex velocity
		1,					//const int isSolid,							//solid or surface 1: solid, 0: surface
		&element[0], nv_per_elm, nelm,	thickness, mtl);
	CSimuSpringObj& s= *ss;
	printf("Spring object spring count is %d\n", s.getElementCount());

	//set constraints
	CVertexConstraints constraints;
	constraints.setFixedPositionXYZ();
	int buffer[]={61, 169, 168, 3, 108, 104, 9, 111, 110, 0, 
				102, 107, 4, 105, 113, 
				6, 174, 106, 8, 180, 109, 10, 112, 176,
				61, 177
	};
	for (int i=0; i<26; i++){
		s.setConstraints(buffer[i], constraints);
	}
	s.setGravityAccelertion(Vector3d(0,-2,0));

	//velocity verlet integration method
	const double dt=0.0005;		//1MS
	CSimuEngine e;
	e.addSimuObject(ss);
	e.setTimestep(dt);

	const int timerid = 0;
	startFastTimer(timerid);
	e.startSimulation(3000, 10);
	stopFastTimer(timerid);
	reportTimeDifference(timerid, "MASSSPRING. run time:");
}


void test_mainROD(void)
{
	typedef CSimuStaticRod T;
	CMeMaterialProperty mtl;
	mtl.setYoung(2.00e5);
	mtl.setPoisson(0.3);
	mtl.setDamping(50.0, 0.0);
	mtl.setMassDamping(1.0e-5);
	int nv=4; 
	int nelm=2;
	const double z1 = 0.02;
	Vector3d pVertex[]={
		Vector3d(2,0,0),  Vector3d(3,0,0), Vector3d(2,0,z1), Vector3d(3,0,z1)
	};		
	Vector3d init_velocity(0,0,0);
	int element[4]={0,1, 2, 3};
	const double rod_crossarea=1e-6;
	T *s = new T(pVertex, nv, init_velocity, element, nelm, rod_crossarea, mtl);	

	//set constraints
	int fixednodes[]={0,2}; 
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
		int movenodes[]={3};
		Vector3d vel(-0.90, 0, -3); vel*=1e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(10, 1, 1000, 1, movenodes, vel);
		//s->addConstraint(velo);
	}

	const double av = PI*1.0*1e-3 *0.999;//0.25000;
	{
		const Vector3d center=pVertex[2];
		const Vector3d norm(0,1,0);
		int vertbuffer[]={3};
		CPlanarRotationConstraint *r = new CPlanarRotationConstraint(
			10, 0, 1000,
			1, vertbuffer, 
			center, norm, av);
		s->addConstraint(r);
	}
	{
		const Vector3d center=pVertex[2];
		const Vector3d norm(0,1,0);
		int vertbuffer[]={3};
		CPlanarRotationConstraint *r = new CPlanarRotationConstraint(
			10, 1001, 1500,
			1, vertbuffer, 
			center, norm, -av);
		//s->addConstraint(r);
	}

	CSimuEngine e;
	e.addSimuObject(s);
	e.setTimestep(1.00e-7); // max step for verlet
	e.startSimulationInMiliSeconds(4000, 20);
	//e.startSimulation(200, 1);
	s->exportElementState(0, mtl, stdout);
	s->exportElementState(1, mtl, stdout);

}


void test_mainTET(void)
{
	typedef CSimuStaticRod T;
	CMeMaterialProperty mtl;
	mtl.setYoung(3.00e5);
	mtl.setPoisson(0.40);
	mtl.setDamping(40.00, 0.0);
	mtl.setMassDamping(1.0e-4);
	int nv=4, nelm=6;
	Vector3d pVertex[]={
		Vector3d(0,0,0), Vector3d(1,0,0),
		Vector3d(0,1,0), Vector3d(0,0,1),	
	};		
	Vector3d init_velocity(0,0,0);
	int element[12]={3,0, 0,1, 1, 2, 2, 0, 3, 1, 3, 2};
	const double rod_crossarea=1e-5;
	T *s = new T(pVertex, nv, init_velocity, element, nelm,	rod_crossarea, mtl);

	//set constraints
	int fixednodes[]={0,1, 2}; 
	const int len = sizeof(fixednodes)/sizeof(int);
	CFixedPositionConstraint* fixpos = new CFixedPositionConstraint(10, 0, 1e10, len, fixednodes);
	s->addConstraint(fixpos);
	s->setGravityAccelertion(Vector3d(0,0,0));
	const double dx = 1;
	const double dy = 0.0;
	const double dz= 0.99;
	{
		int movenodes[]={3};
		Vector3d vel(dx, dy, dz); vel*=1.00e-3;
		CVelocityConstraint *velo = new CVelocityConstraint(10, 1, 1000, 1, movenodes, vel);
		s->addConstraint(velo);
	}
	{
		int movenodes[]={3};
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
	//s->exportElementState(4, mtl, stdout);
	//s->exportElementState(5, mtl, stdout);

}

